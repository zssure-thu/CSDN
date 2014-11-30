using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Dicom;
using Dicom.IO;
using Dicom.Log;
using Dicom.Network;

namespace CFINDScu
{
    class Program
    {
        static void Main(string[] args)
        {
            //构造要发送的C-FIND-RQ消息，如果查看DicomCFindRequest类的话
            //可以看到其定义与DICOM3.0标准第7部分第9章中规定的编码格式一致

            //在构造Study级别的查询时，我们的参数patientID会被填充到消息的Indentifier部分，用来在SCP方进行匹配查询
            var cfind = DicomCFindRequest.CreateStudyQuery(patientId: "12345");


            //当接收到对方发挥的响应消息时，进行相应的操作【注】：该操作在DICOM3.0协议
            //第7部分第8章中有说明，DIMSE协议并未对其做出规定，而应该有用户自己设定
            
            cfind.OnResponseReceived = (rq, rsp) =>
                {
                    //此处我们只是简单的将查询到的结果输出到屏幕
                    Console.WriteLine("PatientAge:{0} PatientName:{1}", rsp.Dataset.Get<string>(DicomTag.PatientAge), rsp.Dataset.Get<string>(DicomTag.PatientName));
                };

            //发起C-FIND-RQ：
            //该部分就是利用A-ASSOCIATE服务来建立DICOM实体双方之间的连接。
            var client = new DicomClient();
            client.AddRequest(cfind);
            client.Send(host:"127.0.0.1",port: 12345,useTls: false,callingAe: "SCU-AE",calledAe: "SCP-AE");
            Console.ReadLine();
        }
    }
}
