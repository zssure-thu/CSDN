using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Dicom;
using Dicom.IO;
using Dicom.Network;
using Dicom.Log;

using System.IO;

namespace CFINDScp
{
    //DICOM3.0协议第7部分第8章中DIMSE协议并未规定请求方和实现方如何来进行具体操作
    //此处定义的DcmCFindCallback代理由用户自己来实现接收到C-FIND-RQ后的操作

    public delegate IList<DicomDataset> DcmCFindCallback(DicomCFindRequest request);

    //要想提供C-FIND SCP服务，需要继承DicomService类，该类中实现了DICOM协议的基础框架，
    //另外还需要实现IDicomCFindProvider接口,用于实现具体的C-FIND SCP服务。
    class ZSCFindSCP : DicomService, IDicomServiceProvider, IDicomCFindProvider
    {
        public ZSCFindSCP(Stream stream,Logger log):base(stream,log)
        {

        }
        #region C-FIND
        public static DcmCFindCallback OnZSCFindRequest;
        public virtual IEnumerable<DicomCFindResponse> OnCFindRequest(DicomCFindRequest request)
        {
            DicomStatus status = DicomStatus.Success;
            IList<DicomDataset> queries;
            List<DicomCFindResponse> responses = new List<DicomCFindResponse>();
            if (OnZSCFindRequest != null)
            {
                //此处通过代理来返回在服务端本机进行的操作结果，也就是DICOM协议中说的匹配查询结果
                queries = OnZSCFindRequest(request);
                if (queries != null)
                {
                    Logger.Info("查询到{0}个数据", queries.Count);

                    foreach (var item in queries)
                    {
                        //对于每一个查询匹配的结果，都需要有一个单独的C-FIND-RSP消息来返回到请求端
                        //【注】：每次发送的状态都必须是Pending，表明后续还会继续发送查询结果
                        DicomCFindResponse rsp = new DicomCFindResponse(request, DicomStatus.Pending);
                        rsp.Dataset = item;
                        responses.Add(rsp);
                    }
                }
                else
                {
                    status = DicomStatus.QueryRetrieveOutOfResources;
                }
            }
            //随后需要发送查询结束的状态，即Success到C-FIND SCU端
            responses.Add(new DicomCFindResponse(request, DicomStatus.Success));
            
            //这里貌似是一起将多个response发送出去的？需要后续在研究一下DicomService中的实现代码
            //搞清楚具体的发送机制
            return responses;
        }
        #endregion

        //下面这部分是A-ASSOCIATE服务的相关实现，此处为了简单只实现了连接请求服务
        #region ACSE-Service

        //A-ASSOCIATE-RQ:
        public void OnReceiveAssociationRequest(DicomAssociation association)
        {


            foreach (var pc in association.PresentationContexts)
            {
                if (pc.AbstractSyntax == DicomUID.Verification)
                    pc.AcceptTransferSyntaxes(AcceptedTransferSyntaxes);
                else
                    if (pc.AbstractSyntax == DicomUID.StudyRootQueryRetrieveInformationModelFIND ||
                        pc.AbstractSyntax == DicomUID.StudyRootQueryRetrieveInformationModelMOVE ||
                        pc.AbstractSyntax == DicomUID.PatientRootQueryRetrieveInformationModelFIND ||
                        pc.AbstractSyntax == DicomUID.PatientRootQueryRetrieveInformationModelMOVE
                        )
                    {
                        //未添加Transfer Syntax限制
                        pc.SetResult(DicomPresentationContextResult.Accept);
                    }
                    else
                        if (pc.AbstractSyntax.StorageCategory != DicomStorageCategory.None)
                            pc.AcceptTransferSyntaxes(AcceptedImageTransferSyntaxes);
            }
            SendAssociationAccept(association);
        }

        //A-RELEASE-RQ
        public void OnReceiveAssociationReleaseRequest()
        {
            SendAssociationReleaseResponse();
        }
        //A-ABORT
        public void OnReceiveAbort(DicomAbortSource source, DicomAbortReason reason)
        {
        }
        //CONNECTION CLOSED
        public void OnConnectionClosed(int errorCode)
        {
        }

        #endregion

        #region Transfer Syntaxes

        private static DicomTransferSyntax[] AcceptedTransferSyntaxes = new DicomTransferSyntax[] {
				DicomTransferSyntax.ExplicitVRLittleEndian,
				DicomTransferSyntax.ExplicitVRBigEndian,
				DicomTransferSyntax.ImplicitVRLittleEndian
			};

        private static DicomTransferSyntax[] AcceptedImageTransferSyntaxes = new DicomTransferSyntax[] {
				// Lossless
				DicomTransferSyntax.JPEGLSLossless,
				DicomTransferSyntax.JPEG2000Lossless,
				DicomTransferSyntax.JPEGProcess14SV1,
				DicomTransferSyntax.JPEGProcess14,
				DicomTransferSyntax.RLELossless,
			
				// Lossy
				DicomTransferSyntax.JPEGLSNearLossless,
				DicomTransferSyntax.JPEG2000Lossy,
				DicomTransferSyntax.JPEGProcess1,
				DicomTransferSyntax.JPEGProcess2_4,

				// Uncompressed
				DicomTransferSyntax.ExplicitVRLittleEndian,
				DicomTransferSyntax.ExplicitVRBigEndian,
				DicomTransferSyntax.ImplicitVRLittleEndian
			};
        #endregion
    }
    class Program
    {
        static void Main(string[] args)
        {
            //模拟一下接收到查询请求后本机的数据库等相关查询操作，即绑定DcmCFindCallback代理
            ZSCFindSCP.OnZSCFindRequest = (request) =>
                {
                    //request中的Identifier字段中包含了SCU希望在SCP端进行匹配查询的信息
                    //我们需要模拟相关操作，此处简单的假设本机中存在满足条件的结果，直接返回

                    IList<DicomDataset> queries = new List<DicomDataset>();

                    //我们此次查询到了三条记录
                    for (int i = 0; i < 3; ++i)
                    {
                        DicomDataset dataset = new DicomDataset();
                        DicomDataset dt = new DicomDataset();
                        dt.Add(DicomTag.PatientID, "20141130");
                        dt.Add(DicomTag.PatientName, "zsure");
                        dt.Add(DicomTag.PatientAge, i.ToString());
                        queries.Add(dt);
                    }
                    return queries;
                };
            var cfindServer = new DicomServer<ZSCFindSCP>(12345);
            //控制台程序，用于确保主程序不退出才可一直提供DICOM C-FIND 服务
            Console.ReadLine();
        }
    }
}
