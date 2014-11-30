using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


using Dicom;
using Dicom.IO;
using Dicom.Network;
using System.IO;
using Dicom.Log;

namespace CMOVEScu
{
    //C-MOVE SCU端需要实现C-STORE SCP服务
    //当然也不一定是C-MOVE SCU端来实现，也可能是第三方来实现C-STORE SCP服务，意思就是说：
    //A向B发送C-MOVE RQ，B接收到C-MOVE-RQ并查询到图像后向C发起C-STORE-RQ，然后C对C-STORE-RQ进行分析并存储图像。



    /// <summary>
    /// 单独实现了C-STORE SCP服务，为C-MOVE SCU做准备
    /// </summary>

    public delegate DicomCStoreResponse OnCStoreRequestCallback(DicomCStoreRequest request);

    class CStoreSCP : DicomService, IDicomServiceProvider, IDicomCStoreProvider, IDicomCEchoProvider
    {
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

        public CStoreSCP(Stream stream, Logger log)
            : base(stream, log)
        {
        }

        public void OnReceiveAssociationRequest(DicomAssociation association)
        {


            foreach (var pc in association.PresentationContexts)
            {
                if (pc.AbstractSyntax == DicomUID.Verification)
                    pc.AcceptTransferSyntaxes(AcceptedTransferSyntaxes);
                else if (pc.AbstractSyntax.StorageCategory != DicomStorageCategory.None)
                    pc.AcceptTransferSyntaxes(AcceptedImageTransferSyntaxes);
            }

            SendAssociationAccept(association);
        }

        public void OnReceiveAssociationReleaseRequest()
        {
            SendAssociationReleaseResponse();
        }

        public void OnReceiveAbort(DicomAbortSource source, DicomAbortReason reason)
        {
        }

        public void OnConnectionClosed(int errorCode)
        {
        }
        public static OnCStoreRequestCallback OnCStoreRequestCallBack;
        public DicomCStoreResponse OnCStoreRequest(DicomCStoreRequest request)
        {
            //to do yourself
            //实现自定义的存储方案
            if (OnCStoreRequestCallBack != null)
            {
                return OnCStoreRequestCallBack(request);
            }
            return new DicomCStoreResponse(request, DicomStatus.NoSuchActionType);
        }

        public void OnCStoreRequestException(string tempFileName, Exception e)
        {
            // let library handle logging and error response
        }

        public DicomCEchoResponse OnCEchoRequest(DicomCEchoRequest request)
        {
            return new DicomCEchoResponse(request, DicomStatus.Success);
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            //开启C-STORE SCP服务，用于接收C-MOVE SCP返回的图像
            CStoreSCP.OnCStoreRequestCallBack = (request) =>
                {
                    var studyUid = request.Dataset.Get<string>(DicomTag.StudyInstanceUID);
                    var instUid = request.SOPInstanceUID.UID;

                    var path = Path.GetFullPath(@"c:\cmove-scu");
                    path = Path.Combine(path, studyUid);

                    if (!Directory.Exists(path))
                        Directory.CreateDirectory(path);

                    path = Path.Combine(path, instUid) + ".dcm";

                    request.File.Save(path);

                    return new DicomCStoreResponse(request, DicomStatus.Success);


                };

            var cstoreServer = new DicomServer<CStoreSCP>(22345);

            //发起C-MOVE-RQ操作,发送请求的StudyID是12
            DicomCMoveRequest req=new DicomCMoveRequest("DEST-AE","12");
            var client=new DicomClient();
            client.NegotiateAsyncOps();
            client.AddRequest(req);
            //这里的IP地址是C-MOVE SCP的地址，12345端口号是C-MOVE SCP提供C-MOVE服务的端口
            //在C-MOVE SCP端发出的C-STORE-RQ子操作请求的是C-MOVE SCU端我们实现的C-STORE SCP，C-STORE SCP绑定的端口是22345
            client.Send("127.0.0.1", 12345,false, "DEST-AE", "SCP-AE");
            Console.ReadLine();

        }
    }
}
