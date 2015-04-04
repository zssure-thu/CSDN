using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;

namespace WCF.WcfContract
{
    [ServiceContract(Name="WcfNotify")]
    public interface INotify2WinForm
    {
        [OperationContract]
        void Notify2WinForm(string message);
    }
}
