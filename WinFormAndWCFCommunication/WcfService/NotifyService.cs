using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using WCF.WcfContract;
using System.ServiceModel;
namespace WCF.WcfService
{
    public delegate void DisplayMessage(string mes);
    [ServiceBehavior(InstanceContextMode=InstanceContextMode.Single)]
    public class NotifyService:INotify2WinForm
    {
        public DisplayMessage displayMessage;
        public void Notify2WinForm(string message)
        {
            if (!string.IsNullOrWhiteSpace(message))
            {
                if (null != displayMessage)
                    displayMessage(message);
            }
        }
    }
}
