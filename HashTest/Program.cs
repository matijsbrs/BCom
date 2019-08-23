using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Toolkits.Networking.IOT;
using Toolkits.Checksum;
using Toolkits;
using Toolkits.IO;
using Toolkits.Networking.M2Mqtt;
using Toolkits.Networking.M2Mqtt.Messages;
using BCMsgCommander;
using System.Web.Script.Serialization;

namespace HashTest
{
    class Program
    {
        private Serial _Serial;
        private MqttClient client;
        BCMsg msg;
        Dictionary<string, BCMsg> NodeBySubscriptionString;
        Dictionary<string, Device.Peripheral> PeripheralList;
        Dictionary<UInt16, BCMsg> NodeBySubscriptionHash;
        Dictionary<UInt32, string> DevicePathTable;

        static void Main(string[] args)
        {
            new Program();
        }

        /// <summary>
        /// Connector method for MQTT broker connection.
        /// </summary>
        public void ConnectToBroker()
        {
            ConsoleEx.State(ConsoleEx.State_e.Start, "Connecting to Mosquitto broker.");
            string BrokerAddress = Properties.Settings.Default.BrokerAddress;
            client = new MqttClient(BrokerAddress);
            client.MqttMsgPublishReceived += MqttMessageReceivedEvent;

            string[] topics = { "#" };
            byte[] qosLevels = { 0 };

            client.Subscribe(topics, qosLevels);
            // use a unique id as client id, each time we start the application
            try
            {
                client.Connect(Guid.NewGuid().ToString());
            }
            catch (Exception e)
            {

            }

            if (client.IsConnected)
                ConsoleEx.State(ConsoleEx.State_e.Ready, "");
            else
            {
                ConsoleEx.State(ConsoleEx.State_e.Error, $"Error connecting to the broker on: '{BrokerAddress}'");
                System.Environment.Exit(-1); // error
            }

        }


        /// <summary>
        /// Mqtt messages received event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e">mqtt payload</param>
        private void MqttMessageReceivedEvent(object sender, MqttMsgPublishEventArgs e)
        {
            List<string> topic = new List<string>(e.Topic.Split('/'));
            try
            {
                if (NodeBySubscriptionString.ContainsKey(e.Topic))
                {
                    //NodeBySubscriptionString[e.Topic].Build(e.Message);
                    //NodeBySubscriptionString[e.Topic].Dump();

                    BCMsg.WebInterfaceClass web = new BCMsg.WebInterfaceClass();
                    web = web.Import(e.Message);

                    if ( web.msgUid == (NodeBySubscriptionString[e.Topic].msgUid) ) {
                        //Console.WriteLine("Dublicate");
                    }
                    else
                    {
                        NodeBySubscriptionString[e.Topic].Operator = web.Operator;
                        NodeBySubscriptionString[e.Topic].Build(web.Value);
                        NodeBySubscriptionString[e.Topic].Dump();
                        Console.WriteLine($"Serial   -> substr: { NodeBySubscriptionString[e.Topic].GetSubscription(),40} ({ NodeBySubscriptionString[e.Topic].GetHash().ToString("X4")}-{ NodeBySubscriptionString[e.Topic].Operator.ToString("X2")}) uid:{ NodeBySubscriptionString[e.Topic].msgUid.ToString("X8")} : { NodeBySubscriptionString[e.Topic].GetStringPayload()}");
                        _Serial.WriteLine(NodeBySubscriptionString[e.Topic].ToBComString());

                    }
                }
            }
            catch (Exception ex)
            {
                ConsoleEx.Error("Check JSon string. Import failed." + ex.ToString());
            }

        }


        


        public void Connect(string Port, int BaudRate)
        {
            _Serial = new Serial(Port, BaudRate);
            _Serial.OnDataReceived += DataReceivedHandler;
            _Serial.Open();

        }
        private Random rand = new Random();
        private void DataReceivedHandler(object sender, SerialDataArgs e)
        {

            if (e.ToString().StartsWith("#") )
            {
                try
                {
                    byte[] msgIn = Convert.FromBase64String(e.ToString().TrimEnd(new char[] { '\n', '\r' }).TrimStart(new char[] { '#' }));

                    UInt16 Subscriber = (UInt16)((msgIn[0] << 8) | (msgIn[1]));
                    byte Operator = msgIn[2];
                    byte[] Payload = msgIn.GetRange(3, msgIn.Length - 3);

                    Console.WriteLine($"req {Subscriber.ToString("X4")}");

                    if (NodeBySubscriptionHash.ContainsKey(Subscriber))
                    {
                        NodeBySubscriptionHash[Subscriber].Operator = Operator;
                        NodeBySubscriptionHash[Subscriber].Build(Payload);
                        NodeBySubscriptionHash[Subscriber].EventGenerator();
                        //NodeBySubscriptionHash[Subscriber].Dump();
                        NodeBySubscriptionHash[Subscriber].msgUid = rand.Next();
                        Console.WriteLine($"mqtt pub -> substr: {NodeBySubscriptionHash[Subscriber].GetSubscription(),40} ({Subscriber.ToString("X4")}-{Operator.ToString("X2")}) uid:{NodeBySubscriptionHash[Subscriber].msgUid.ToString("X8")} : {NodeBySubscriptionHash[Subscriber].GetStringPayload()} ");

                        BCMsg.WebInterfaceClass web = new BCMsg.WebInterfaceClass(NodeBySubscriptionHash[Subscriber]);
                        client.Publish($"{web.subscriber}", web.Export().ToByteArr());

                    }
                } catch (FormatException Fe)
                {
                    ConsoleEx.Error("Malformed BC Message, possible collision");
                }
            }
        }

        public class LampController : Device
        {
            public Relay relay;

            public class Relay : Peripheral
            {
                public Relay(Device parentDeviceIn, string nameIn) : base(parentDeviceIn, nameIn)
                {

                }
                public void Set()
                {
                    Push(new byte[] { 0x01 });
                }
                public void Reset()
                {
                    Push(new byte[] { 0x00 });
                }
            }

            public class Subscriber : Peripheral
            {
                public Subscriber(Device parentDeviceIn, string nameIn) : base(parentDeviceIn, nameIn)
                {
                    msg.OnPullEvent += Msg_OnPullEvent;
                }

                private void Msg_OnPullEvent(object sender, BCMsg.BCMsgArgs e)
                {
                    ConsoleEx.Notice($"Device {parentDevice.Path} is ready");
                    foreach (Peripheral peripheral in parentDevice.Peripherals)
                    {
                        ConsoleEx.Notice($"\t{peripheral.Name} {peripheral.LinkedPeripherals.Count() }");
                        foreach (Peripheral linkedperipheral in peripheral.LinkedPeripherals)
                        {
                            ConsoleEx.Notice($"\tlinked to: {linkedperipheral.Name} {linkedperipheral.GetPathHash().ToString("X4")}");
                            byte[] tmp = new byte[] {(byte) (peripheral.NameHash>>8), (byte) peripheral.NameHash, (byte) (linkedperipheral.GetPathHash()>>8), (byte)(linkedperipheral.GetPathHash()) };
                            Push(tmp);
                        }
                    }
                }
            }

            public LampController(MqttClient client,  UInt32 uidIn, string pathIn) : base(client, uidIn, pathIn)
            {
                relay = new Relay(this, "relay");
                AddPeripheral(relay);
                AddPeripheral(new Subscriber(this, "subscribe"));
            }
        }
        
        public class DeviceManager : Device
        {
            public Register register;
            protected Dictionary<string, BCMsg> NodeBySubscriptionString;
            protected Dictionary<UInt16, BCMsg> NodeBySubscriptionHash;
            public Dictionary<UInt32, string> DevicePathTable;

            public class Register : Peripheral
            {
                public Register(Device parentDeviceIn, string nameIn) : base(parentDeviceIn, nameIn)
                {
                    msg.OnPushEvent += Register_OnPushEvent;
                   
                }

                private void Register_OnPushEvent(object sender, BCMsg.BCMsgArgs e)
                {
                    BCMsg _msg = (sender as BCMsg);
                    try
                    {
                        UInt32 RegisteringDeviceUID = _msg.GetPayload().Reverse().ToUInt32();
                        Console.WriteLine($"Hello device: {RegisteringDeviceUID.ToString("X4")}");
                        _msg.Dump();


                        if ((parentDevice as DeviceManager).DevicePathTable.ContainsKey(RegisteringDeviceUID))
                        {
                            Console.WriteLine($"Hello device: {RegisteringDeviceUID.ToString("X4")}");
                            UInt16 PathHash = Crc16.ComputeChecksum(((parentDevice as DeviceManager).DevicePathTable[RegisteringDeviceUID] + "/").ToByteArr());
                            Console.WriteLine($"PathHash {PathHash.ToString("X4"),6} Path: {(parentDevice as DeviceManager).DevicePathTable[RegisteringDeviceUID] + "/" }");
                            byte[] TempArr = { (byte)(RegisteringDeviceUID >> 24), (byte)(RegisteringDeviceUID >> 16), (byte)(RegisteringDeviceUID >> 8), (byte)(RegisteringDeviceUID), (byte)(PathHash >> 8), (byte)PathHash };
                            msg.Build(TempArr);
                            //msg.msgUid = rand.Next();
                            msg.Command = BCMsg.Commande.PUSH;
                            msg.Dump();
                            BCMsg.WebInterfaceClass web = new BCMsg.WebInterfaceClass(msg);
                            //client.Publish($"{web.subscriber}", web.Export().ToByteArr());
                            parentDevice.client.Publish($"{web.subscriber}", web.Export().ToByteArr());
                            
                        }
                        else
                        {
                            Console.WriteLine($"Unknown device: {msg.GetPayload().ToHexString()}");
                        }
                    }
                    catch (Exception err)
                    {
                        ConsoleEx.Error($"Could not register");
                    }
                }
            }
            public DeviceManager(MqttClient client, UInt32 uidIn, string pathIn) : base(client, uidIn, pathIn)
            {
                register = new Register(this, "register");
                AddPeripheral(register);
            }

            public void addHashTables(Dictionary<string, BCMsg> nodeBySubscriptionStringIn, Dictionary<UInt16, BCMsg> nodeBySubscriptionHashIn, Dictionary<UInt32, string> devicePathTableIn)
            {
                NodeBySubscriptionHash = nodeBySubscriptionHashIn;
                NodeBySubscriptionString = nodeBySubscriptionStringIn;
                DevicePathTable = devicePathTableIn;
            }
        }

        public void AddDeviceToHashTable(Device _device)
        {
            if (_device.Uid == 0)
            {
                _device.Uid = (uint)_device.GetHashCode();
                ConsoleEx.Notice($"Zero device added: assigning {_device.Uid} to {_device.Path}");
            }
            DevicePathTable.Add(_device.Uid, _device.Path);
            foreach (Device.Peripheral peripheral in _device.Peripherals)
            {
                NodeBySubscriptionString.Add(peripheral.GetPath(), peripheral.msg);
                NodeBySubscriptionHash.Add(peripheral.GetPathHash(), peripheral.msg);
                //PeripheralList.Add(peripheral.GetPath(), peripheral);
            }
        }


        public class Marvin
        {
            private List<Device> devices;

            public Device[] Devicelist {
                get
                {
                    return devices.ToArray();
                }
                set
                {
                    devices.AddRange(value);
                }
                
            }

            public Marvin()
            {
                devices = new List<Device>();
            }

            public Marvin Import(byte[] input)
            {
                JavaScriptSerializer _JSS = new JavaScriptSerializer();
                Marvin WIc = _JSS.Deserialize<Marvin>(Encoding.UTF8.GetString(input));
                return WIc;
            }

            public string Export()
            {
                JavaScriptSerializer _JSS = new JavaScriptSerializer();
                return _JSS.Serialize(this);
            }

            public void Add(Device deviceIn)
            {
                devices.Add(deviceIn);
            }
        }

        public Program()
        {

            BCom bcom = new BCom();
            Marvin marvin = new Marvin();
            
            NodeBySubscriptionString = new Dictionary<string, BCMsg>();
            NodeBySubscriptionHash = new Dictionary<ushort, BCMsg>();
            DevicePathTable = new Dictionary<uint, string>();
          
            Connect(Properties.Settings.Default.ComPort, 115200);
            ConnectToBroker();

            LampController lc1 = new LampController(client, 0x01945431, "huis/tuin/Lc");
            LampController lc2 = new LampController(client, 0x01945230, "work/desk/1/Lc");
            LampController lc3 = new LampController(client, 0x0F947A10, "work/desk/2/Lc");
            LampController lcWork = new LampController(client, 0x00000000, "group/Lc");

            LampController toren1 = new LampController(client, 0x0194540D, "toren/1");
            LampController toren2 = new LampController(client, 0x0F942418, "toren/2");
            LampController toren3 = new LampController(client, 0x01945B2A, "toren/3");
            LampController toren4 = new LampController(client, 0x0F942423, "toren/4");
            LampController toren5 = new LampController(client, 0x0F943E29, "toren/5");
            LampController toren6 = new LampController(client, 0x0F942E39, "toren/6");
            LampController toren7 = new LampController(client, 0x13D42C64, "toren/7");
            LampController toren8 = new LampController(client, 0x01945418, "toren/8");
            LampController toren9 = new LampController(client, 0x01945429, "toren/9");
            LampController toren10 = new LampController(client, 0x01945B1C, "toren/10");
            LampController toren11 = new LampController(client, 0x0F942F2F, "toren/11");
            LampController toren12 = new LampController(client, 0x0F942420, "toren/12");
            LampController toren13 = new LampController(client, 0x0F945A30, "toren/13");
            LampController toren14 = new LampController(client, 0x0F94241C, "toren/14");

            LampController toren = new LampController(client, 0x00000000, "toren/");

            toren1.relay.LinkedPeripherals.Add(toren.relay);
            toren2.relay.LinkedPeripherals.Add(toren.relay);
            toren3.relay.LinkedPeripherals.Add(toren.relay);
            toren4.relay.LinkedPeripherals.Add(toren.relay);
            toren5.relay.LinkedPeripherals.Add(toren.relay);
            toren6.relay.LinkedPeripherals.Add(toren.relay);
            toren7.relay.LinkedPeripherals.Add(toren.relay);
            toren8.relay.LinkedPeripherals.Add(toren.relay);
            toren9.relay.LinkedPeripherals.Add(toren.relay);
            toren10.relay.LinkedPeripherals.Add(toren.relay);
            toren11.relay.LinkedPeripherals.Add(toren.relay);
            toren12.relay.LinkedPeripherals.Add(toren.relay);
            toren13.relay.LinkedPeripherals.Add(toren.relay);
            toren14.relay.LinkedPeripherals.Add(toren.relay);

            marvin.Add(toren1);
            marvin.Add(toren2);
            marvin.Add(toren3);
            marvin.Add(toren4);
            marvin.Add(toren5);
            marvin.Add(toren6);
            marvin.Add(toren7);
            marvin.Add(toren8);
            marvin.Add(toren9);
            marvin.Add(toren10);
            marvin.Add(toren11);
            marvin.Add(toren12);
            marvin.Add(toren13);
            marvin.Add(toren14);
            marvin.Add(toren);

            marvin.Add(lc1);
            marvin.Add(lc2);
            marvin.Add(lc3);
            marvin.Add(lcWork);

            

            AddDeviceToHashTable(toren1);
            AddDeviceToHashTable(toren2);
            AddDeviceToHashTable(toren3);
            AddDeviceToHashTable(toren4);
            AddDeviceToHashTable(toren5);
            AddDeviceToHashTable(toren6);
            AddDeviceToHashTable(toren7);
            AddDeviceToHashTable(toren8);
            AddDeviceToHashTable(toren9);
            AddDeviceToHashTable(toren10);
            AddDeviceToHashTable(toren11);
            AddDeviceToHashTable(toren12);
            AddDeviceToHashTable(toren13);
            AddDeviceToHashTable(toren14);
            AddDeviceToHashTable(toren);

            lc1.AddSubscription(lcWork);

            DeviceManager deviceManager = new DeviceManager(client, 0x00001234, "system/network");

            marvin.Add(deviceManager);

            AddDeviceToHashTable(lc1);
            AddDeviceToHashTable(lc2);
            AddDeviceToHashTable(lc3);
            AddDeviceToHashTable(lcWork);

            AddDeviceToHashTable(deviceManager);

            deviceManager.addHashTables(NodeBySubscriptionString, NodeBySubscriptionHash, DevicePathTable);

            lc1.relay.Reset();
            lc3.relay.LinkedPeripherals.Add(lc1.relay);
            lc2.relay.LinkedPeripherals.Add(lc1.relay);

            lc1.relay.LinkedPeripherals.Add(lcWork.relay);
            lc2.relay.LinkedPeripherals.Add(lcWork.relay);
            lc3.relay.LinkedPeripherals.Add(lcWork.relay);

            //Console.WriteLine(marvin.Export());


            Console.ReadLine();
        }
    }
}
