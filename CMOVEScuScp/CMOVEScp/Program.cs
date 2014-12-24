using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Dicom;
using Dicom.IO;
using Dicom.Network;
using Dicom.Log;
using System.IO;

namespace CMOVEScp
{
    //DICOM3.0协议第7部分第8章中DIMSE协议并未规定请求方和实现方如何来进行具体操作
    //此处定义的DcmCFindCallback代理由用户自己来实现接收到C-FIND-RQ后的操作

    public delegate IList<DicomDataset> DcmCMoveCallback(DicomCMoveRequest request);

    //要想提供C-FIND SCP服务，需要继承DicomService类，该类中实现了DICOM协议的基础框架，
    //另外还需要实现IDicomCFindProvider接口,用于实现具体的C-FIND SCP服务。
    class ZSCMoveSCP : DicomService, IDicomServiceProvider, IDicomCMoveProvider
    {
        public ZSCMoveSCP(Stream stream, Logger log)
            : base(stream, log)
        {

        }
        #region C-MOVE
        public static DcmCMoveCallback OnZSCMoveRequest;
        public virtual IEnumerable<DicomCMoveResponse> OnCMoveRequest(DicomCMoveRequest request)
        {
            DicomStatus status = DicomStatus.Success;
            IList<DicomCMoveResponse> rsp = new List<DicomCMoveResponse>();
            /*----to do------*/
            //添加查询数据库的代码，即根据request的条件提取指定的图像
            //然后将图像信息添加到rsp响应中

            //创建C-STORE-SCU，发起C-STORE-RQ
            IList<DicomDataset> queries;
            DicomClient clt = new DicomClient();

            if (OnZSCMoveRequest != null)
            {
                queries = OnZSCMoveRequest(request);
                if (queries != null)
                {
                    Logger.Info("需要发送{0}个数据", queries.Count);
                    int len = queries.Count;
                    int cnt = 0;
                    foreach (var item in queries)
                    {
                        //zssure：
                        //取巧的方法直接利用request来构造response中相同的部分
                        //这部分与mDCM方式很不同
                        var studyUid = item.Get<string>(DicomTag.StudyInstanceUID);
                        var instUid = item.Get<string>(DicomTag.SOPInstanceUID);
                        //需要在c:\cmovetest目录下手动添加C-MOVE SCU请求的图像
                        //本地构造的目录结构为，
                        //      c:\cmovetest\12\0.dcm
                        //      c:\cmovetest\12\1.dcm
                        //      c:\cmovetest\12\2.dcm
                        var path = Path.GetFullPath(@"c:\cmovetest");

                        try
                        {
                            path = Path.Combine(path, studyUid);
                            if (!Directory.Exists(path))
                                Directory.CreateDirectory(path);
                            path = Path.Combine(path, instUid) + ".dcm";
                            DicomCStoreRequest cstorerq = new DicomCStoreRequest(path);
                            cstorerq.OnResponseReceived = (rq, rs) =>
                            {
                                if (rs.Status != DicomStatus.Pending)
                                {

                                }
                                if (rs.Status == DicomStatus.Success)
                                {
                                    DicomCMoveResponse rsponse = new DicomCMoveResponse(request, DicomStatus.Pending);
                                    rsponse.Remaining = --len;
                                    rsponse.Completed = ++cnt;
                                    rsponse.Warnings = 0;
                                    rsponse.Failures = 0;
                                    //zssure:2014-12-24
                                    //修复发送C-MOVE-RSP的逻辑错误
                                    SendResponse(rsponse);
                                    //rsp.Add(rsponse);
                                    //zssure:end

                                }

                            };
                            clt.AddRequest(cstorerq);
                            //注意：这里给出的IP地址与C-MOVE请求的IP地址相同，意思就是说C-MOVE SCP需要向C-MOVE SCU发送C-STORE-RQ请求
                            //将查询到的图像返回给C-MOVE SCU
                            //所以四尺C-STORE-RQ中的IP地址与C-MOVE SCU相同，但是端口不同，因为同一个端口不能被绑定多次。
                            clt.Send("127.0.0.1", 22345, false, this.Association.CalledAE, request.DestinationAE);

                        }
                        catch (System.Exception ex)
                        {
                            DicomCMoveResponse rs = new DicomCMoveResponse(request, DicomStatus.StorageStorageOutOfResources);
                            rsp.Add(rs);
                            return rsp;

                        }
                    }
                    //zssure:
                    //发送完成后统一返回C-MOVE RESPONSE 
                    //貌似响应流程有问题，有待进一步核实
                    //注意，如果最后为发送DicomStatus.Success消息，TCP连接不会释放，浪费资源
                    rsp.Add(new DicomCMoveResponse(request, DicomStatus.Success));
                    return rsp;

                }
                else
                {
                    rsp.Add(new DicomCMoveResponse(request, DicomStatus.NoSuchObjectInstance));
                    return rsp;

                }

            }
            rsp.Add(new DicomCMoveResponse(request, DicomStatus.NoSuchObjectInstance));
            return rsp;

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
            ZSCMoveSCP.OnZSCMoveRequest = (request) =>
            {
                List<DicomDataset> dataset = new List<DicomDataset>();
                for (int i = 0; i < 3; ++i)
                {
                    DicomDataset dt = new DicomDataset();
                    dt.Add(DicomTag.StudyInstanceUID, "12");
                    dt.Add(DicomTag.SOPInstanceUID, i.ToString());
                    dataset.Add(dt);

                }
                return dataset;
            };
            var cmoveScp = new DicomServer<ZSCMoveSCP>(12345);
            Console.ReadLine();

        }
    }
}
