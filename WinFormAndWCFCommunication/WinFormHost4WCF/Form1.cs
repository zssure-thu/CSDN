using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using WCF.WcfContract;
using WCF.WcfService;
using System.ServiceModel;
namespace WinFormHost4WCF
{
    public partial class Form1 : Form
    {
        private ServiceHost host = null;
        public Form1()
        {
            InitializeComponent();
            button1.Enabled = true;
            button2.Enabled = false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            NotifyService wcfService = new NotifyService();
            wcfService.displayMessage = this.DisplayMessage;
            host = new ServiceHost(wcfService);
            WSHttpBinding httpBinding = new WSHttpBinding(SecurityMode.None);
            host.AddServiceEndpoint(typeof(INotify2WinForm), httpBinding, "http://localhost:8900");
            NetTcpBinding tcpBinding = new NetTcpBinding(SecurityMode.None);
            host.AddServiceEndpoint(typeof(INotify2WinForm), tcpBinding, "net.tcp://localhost:1700/");
            host.Open();
            button1.Enabled = false;
            button2.Enabled = true;
        }

        public void DisplayMessage(string mes)
        {
            MessageBox.Show(mes);
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
        private void Form1_FormClosing(object sender, EventArgs e)
        {
            base.OnClosed(e);
            if (host != null)
                host.Close();

        }
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (host != null)
                host.Close();
            button2.Enabled = false;
            button1.Enabled = true;
        }
    }
}
