using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.ServiceModel;
using WCF.WcfContract;
namespace WinformClient4WcfTest
{
    public partial class Form1 : Form
    {
        EndpointAddress edpHttp = new EndpointAddress("http://localhost:8900/");
        EndpointAddress edpTcp = new EndpointAddress("net.tcp://localhost:1700/");
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
            ChannelFactory<INotify2WinForm> factory = new ChannelFactory<INotify2WinForm>(httpBinding);
            INotify2WinForm channel = factory.CreateChannel(edpHttp);

            channel.Notify2WinForm(textBox1.Text);
            ((IClientChannel)channel).Close();
        }
    }
}
