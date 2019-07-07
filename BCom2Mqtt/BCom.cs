using System;
using System.Collections.Generic;
using System.Web.Script.Serialization;
using System.IO.Ports;
using Toolkits;
using Toolkits.Checksum;
using Toolkits.Networking.M2Mqtt;
using Toolkits.Networking.M2Mqtt.Messages;
using System.Text;

namespace BCom2Mqtt
{
    /// <summary>
    /// Event class
    /// </summary>
    public class DataArgs : EventArgs
    {
        public BComFrame frame;
        public DataArgs(BComFrame comFrame)
        {
            frame = comFrame;
        }
    }

    public class BComFrame
    {
        #region Variables
            /// <summary>
            /// This is used to store the fully processed Base64 encoded frame
            /// </summary>
        private string _base64FrameString;
            /// <summary>
            /// The complete frame encoded as a byte array.
            /// The array is private to exclude it from the import and export methods
            /// </summary>
        private byte[] _byteArrFrame;
            /// <summary>
            /// used for JSon import routines
            /// </summary>
        private byte[] importPayload;
            /// <summary>
            /// Target BCom ID
            /// </summary>
        public UInt32 Target;
            /// <summary>
            /// The BCom ID of this application
            /// </summary>
        public UInt32 Source;
            /// <summary>
            /// This byte contains the frame settings. control this using the properties AckReq,Ack,Nack,Encrypted And Ping
            /// </summary>
        public byte system;
            /// <summary>
            /// The port number used to select the end application.
            /// </summary>
        public byte port;
            /// <summary>
            /// The frame counter
            /// </summary>
        public UInt16 FCount;
            /// <summary>
            /// The Crc value of the frame
            /// </summary>
        UInt16 Crc;
        #endregion

        #region Control Properties
        public bool AckReq
    {
        get
        {
            return ((system & 0x01) == 0x01);
        }
        set
        {
            system |= 0x01;
        }
    }
        public bool Ack
        {
            get
            {
                return ((system & 0x02) == 0x02);
            }
            set
            {
                system |= 0x02;
            }
        }
        public bool Nack
        {
            get
            {
                return ((system & 0x04) == 0x04);
            }
            set
            {
                system |= 0x04;
            }
        }
        public bool Encrypted
        {
            get
            {
                return ((system & 0x08) == 0x08);
            }
            set
            {
                system |= 0x08;
            }
        }
        public bool Ping
    {
        get
        {
            return ((system & 0x10) == 0x10);
        }
        set
        {
            system |= 0x10;
        }
    }
        public byte[] Payload
        {
            get
            {
                return GetPayload();
            }
            set
            {
                importPayload = value;
            }
        }
        public bool IsValid => (Crc == GetCrc16()) ? true : false;
        #endregion

        #region Constructors
        public BComFrame()
        {

        }

        public BComFrame(UInt32 target, UInt32 source, byte system, byte port, UInt16 fCount, byte[] payload) 
        {
            List<byte> newFrame = new List<byte>();
            //newFrame.Add((byte)':');
            newFrame.AddRange(target.ToByteArr());
            newFrame.AddRange(source.ToByteArr());
            newFrame.Add(system);
            newFrame.Add(port);
            newFrame.AddRange(fCount.ToByteArr());
            newFrame.AddRange(payload);
            newFrame.AddRange(Crc16.ComputeChecksum(newFrame.ToArray().GetRange(0, newFrame.ToArray().Length -1)).ToByteArr());
            _byteArrFrame = newFrame.ToArray();
            _base64FrameString = Convert.ToBase64String(_byteArrFrame);
            ProcessFromString(_base64FrameString);
        }
        public BComFrame(string base64Frame)
        {
            ProcessFromString(base64Frame);
        }
        #endregion

        public string GetBase64FrameString()
        {
            return _base64FrameString;
        }
        
        private void ProcessFromString(string base64Frame)
        {
            _base64FrameString = base64Frame;
            _byteArrFrame = Convert.FromBase64String(base64Frame);
            Target = _byteArrFrame.GetRange(0, 4).ToUInt32();
            Source = _byteArrFrame.GetRange(4, 8).ToUInt32();
            system = _byteArrFrame.GetRange(8, 9)[0];
            port = _byteArrFrame.GetRange(9, 10)[0];
            FCount = _byteArrFrame.GetRange(10, 12).ToUInt16();
            Crc = _byteArrFrame.GetRange(_byteArrFrame.Length - 2, _byteArrFrame.Length).ToUInt16();
        }

        public string GetStringPayload() => _byteArrFrame.GetRange(12, _byteArrFrame.Length - 3).ToHexString();

        public byte[] GetPayload() => _byteArrFrame.GetRange(12, _byteArrFrame.Length - 3);

        public UInt16 GetCrc16() => Crc16.ComputeChecksum(_byteArrFrame.GetRange(0, _byteArrFrame.Length - 3));

        #region Json Import and Export methods
        public string Export()
        {
            JavaScriptSerializer _JSS = new JavaScriptSerializer();
            return _JSS.Serialize(this);
        }

        public BComFrame Import(byte[]  input)
        {
            JavaScriptSerializer _JSS = new JavaScriptSerializer();
            BComFrame bcf = _JSS.Deserialize<BComFrame>(Encoding.UTF8.GetString(input));
            
            return bcf;
        }

        public BComFrame Rebuild()
        {
            List<byte> newFrame = new List<byte>();
            newFrame.AddRange(Target.ToByteArr());
            newFrame.AddRange(Source.ToByteArr());
            newFrame.Add(system);
            newFrame.Add(port);
            newFrame.AddRange(FCount.ToByteArr());
            newFrame.AddRange(importPayload);
            newFrame.AddRange(Crc16.ComputeChecksum(newFrame.ToArray().GetRange(0, newFrame.ToArray().Length - 1)).ToByteArr());
            _byteArrFrame = newFrame.ToArray();
            _base64FrameString = Convert.ToBase64String(_byteArrFrame);
            return this;
        }
        #endregion

        public void Dump()
        {
            Console.WriteLine($"From  : {Source.ToString("X8")} To: {Target.ToString("X8")}");
            Console.WriteLine($"Port  : {port}");
            Console.WriteLine($"Fcount: {FCount}");
            Console.WriteLine($"Payld : {GetStringPayload()} [{System.Text.Encoding.Default.GetString(GetPayload())}]");
            Console.WriteLine($"CRC   : {Crc}");
            Console.WriteLine($"Valid : {IsValid}");
        }
        

    }

    /// <summary>
    /// BCom Serial connector
    /// </summary>
    public class BCom
    {
        public delegate void DataReceived(Object sender, DataArgs e);
        public event DataReceived OnDataReceived = null;

        SerialPort _serialPort;

        public BCom()
        {

        }

        public void Setup(string PortName, int BaudRate)
        {
            // Create a new SerialPort object with default settings.
            _serialPort = new SerialPort();
            _serialPort.PortName = PortName;
            _serialPort.Parity = Parity.None;           
            _serialPort.DataBits = 8;                   
            _serialPort.StopBits = StopBits.One;        
            _serialPort.Handshake = Handshake.None;     
            _serialPort.BaudRate = BaudRate;

            // Set the read/write timeouts
            _serialPort.ReadTimeout = 100;
            _serialPort.WriteTimeout = 60000;
            _serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);


            int i;
            for (i = 0; i < 5; i++)
            {
                try
                {
                    _serialPort.Open();
                    _serialPort.DiscardInBuffer();
                    break;
                }
                catch (System.Exception)
                {
                    Console.WriteLine(@"Could not open Comm port." + _serialPort.PortName);

                }
            }
            if (i == 5) return;
        }


        string payload;
        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            try
            {   // @140219 ^MB starting the timeout thread first.
                string message = _serialPort.ReadLine();
                if (message.StartsWith(":") || message.StartsWith(";") || message.StartsWith("!"))
                {
                    OnDataReceived?.Invoke(this, new DataArgs(new BComFrame(message.Substring(1))));
                }
            }
            catch (TimeoutException te)
            {
                ConsoleEx.Warning("BCom bus transmission to slow. Timeout during reception. Frame discarded");
                payload = "";
                
            }
            catch (FormatException fe)
            {
                ConsoleEx.Warning("Discarded malformed BCom Frame.");
                payload = "";

            }
        }

        void Reconnect()
        {
            
            try
            {
                _serialPort.Open();
                _serialPort.DiscardInBuffer();
            }
            catch (Exception e)
            {
                ConsoleEx.Notice("Reconnecting to Serialport failed.");
            }
            
        }

        internal void Transmit(string v)
        {
            try
            {
                _serialPort.Write($":{v}\n");
            }
            catch (Exception)
            {
                ConsoleEx.Error("Lost connection");
                Reconnect();
            }
        }
    }

    public class BComNetworkLayer
    {

        public delegate void DataReceived(Object sender, DataArgs e);

        public Dictionary<int, DataReceived> PortList = new Dictionary<int, DataReceived>();

        UInt32 MyAddress;
        UInt16 Counter = 0;
        public List<UInt32> MulticastList = new List<UInt32>();
        private BCom bcom;

        public BComNetworkLayer(BCom bcom)
        {
            this.bcom = bcom;
        }

        public UInt32 SetAddress(UInt32 myNetWorkAddress)
        {
            MyAddress = myNetWorkAddress & 0x3FFFFFFF;
            return MyAddress;
        }

        public void Bind(int port, DataReceived dataReceivedEvent)
        {
            PortList.Add(port, dataReceivedEvent);
        }

        public void Bcom_OnDataReceived(object sender, DataArgs e)
        {
            try
            {
                PortList[e.frame.port].Invoke(this, e);
            }
            catch (Exception)
            {

            }
        }

        public void TransmitAsync(int port, UInt32 Address, byte[] payload)
        {
            BComFrame bcf = new BComFrame(Address, MyAddress, 0, (byte)port, Counter++, payload);
            bcom.Transmit(bcf.GetBase64FrameString());
        }

        public void TransmitAsync(BComFrame bComFrame)
        {
            bcom.Transmit(bComFrame.GetBase64FrameString());
        }
    }

}



