using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using Toolkits;
using Toolkits.Networking.M2Mqtt;
using Toolkits.Networking.M2Mqtt.Messages;
using Toolkits.IO;

namespace BCom2Mqtt
{
    class Program
    {
        BComNetworkLayer Bcs;
        MqttClient client;

        /// <summary>
        /// Connector method for MQTT broker connection.
        /// </summary>
        public void ConnectToBroker()
        {
            ConsoleEx.State(ConsoleEx.State_e.Start, "Connecting to Mosquitto broker.");
            string BrokerAddress = Properties.Settings.Default.BrokerAddres;
            client = new MqttClient(BrokerAddress);
            client.MqttMsgPublishReceived += MqttMessageReceivedEvent;

            string[] topics = { "BCom/+/tx" };
            byte[] qosLevels = { 0 };

            client.Subscribe(topics, qosLevels);
            // use a unique id as client id, each time we start the application
            client.Connect(Guid.NewGuid().ToString());
            if (client.IsConnected)
                ConsoleEx.State(ConsoleEx.State_e.Ready, "");
            else
                ConsoleEx.State(ConsoleEx.State_e.Error, "");

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
                if (topic.First().Equals("BCom") && topic.Last().Equals("tx"))
                {
                    string[] topics = e.Topic.Split('/');
                    BComFrame bcf = new BComFrame();
                    Bcs.TransmitAsync(bcf.Import(e.Message).Rebuild());
                }
            }
            catch (Exception ex )
            {
                ConsoleEx.Error("Check JSon string. Import failed." + ex.ToString());
            }
            
        }

        public Program()
        {
            ConnectToBroker();

            BCom bcom = new BCom();
            Bcs = new BComNetworkLayer(bcom);
            bcom.Connect(Properties.Settings.Default.CommPort, 115200);
            bcom.OnDataReceived += OnBComDataReceived;

            string version = Assembly.GetExecutingAssembly().GetName().Version.ToString();

            ConsoleEx.State(ConsoleEx.State_e.Ready, $"{Assembly.GetExecutingAssembly().GetName().Name } v{version}");

            //var backgroundScheduler = TaskScheduler.Default;
            //Task.Factory.StartNew(delegate { Toggle(Bcs); }, backgroundScheduler);
            //.ContinueWith(delegate { CountDown(100); }, backgroundScheduler)
            //.ContinueWith(delegate { ShowText("Test"); }, backgroundScheduler)
            //.ContinueWith(delegate { ShowText("Ready"); }, backgroundScheduler);

            Console.ReadLine();
        }

        /// <summary>
        /// Process a received BCom frame
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnBComDataReceived(object sender, DataArgs e)
        {
            e.frame.Dump();
            client.Publish($"BCom/{e.frame.Source.ToString("X8")}/rx", e.frame.Export().ToByteArr());
        }

        static void Main(string[] args)
        {
            new Program();
        }


    }
}
