package changeUID4Dcm.utils;
import org.dcm4che2.data.*;
import org.dcm4che2.io.*;
//import org.omg.CORBA.PUBLIC_MEMBER;

//import java.awt.List;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
//import java.util.HashSet;
//import java.util.Iterator;
//import java.util.Map;
//import java.util.Set;
//import java.util.concurrent.Executor;
public class AutoChangeDicomUID {

	//private HashMap<String, ArrayList<String>> studyUIDMappingSeriesUIDs=new HashMap<String, ArrayList<String>>();
	//private HashMap<String, ArrayList<String>> seriesUIDMappingImageUIDs=new HashMap<String, ArrayList<String>>();
	private HashMap<String, String> studyUIDBeforeAndAfter=new HashMap<String, String>();
	private HashMap<String, String> serieUIDBeforeAndAfter=new HashMap<String, String>();
	public ArrayList<String> dcmFiles4Changed=new ArrayList<String>();
	public AutoChangeDicomUID() {
		// TODO Auto-generated constructor stub
	
	}
	/*
	 * 遍历文件夹内部的所有文件
	 */
	public final void scanDirectory(String dirPath)
	{
		//ArrayList<String> dcmFiles=new ArrayList<String>();
		if(dirPath.isEmpty())
			return ;
		File root=new File(dirPath);
		if(root.isDirectory())
		{
			File[] files=root.listFiles();
			if(files.length==0)
				return;
			for(File file:files)
			{
				if(file.isDirectory())
					scanDirectory(file.getPath());
				else {
					dcmFiles4Changed.add(file.getPath());
				}
			}
		}
		else {
			dcmFiles4Changed.add(root.getPath());
		}
		
		//return dcmFiles;
	}
	/*
	 * 修改所有DCM文件中的三级UID
	 */
	public final void changeAllUIDs()
	{
		for(String dcmFile : dcmFiles4Changed)
		{
			changeUIDs4SingleDcmFile(dcmFile);
		}
	}
	public final void changeUIDs4SingleDcmFile(String dcmFile)
	{
		DicomObject dcmObj;
		DicomInputStream din=null;
		File fileDCM=new File(dcmFile);
		try {
			din=new DicomInputStream(fileDCM);
			if(din==null)
				return;
			dcmObj=din.readDicomObject();
			//修改StudyUID
			String beforeStudyUID=dcmObj.getString(Tag.StudyInstanceUID);//get Study Instance UID
			if(beforeStudyUID==null)
				beforeStudyUID="1.2.3.4.5.6.7.8.9.0";
			if(!studyUIDBeforeAndAfter.containsKey(beforeStudyUID))
			{
				dcmObj.remove(Tag.StudyInstanceUID);
				SimpleDateFormat dateFormat=new SimpleDateFormat("yyyyMMdd.HHmmss");
				
				String afterStudyUID=String.format("%s.%s", beforeStudyUID,dateFormat.format(new Date()));
				dcmObj.putString(Tag.StudyInstanceUID, VR.UI, afterStudyUID);
				studyUIDBeforeAndAfter.put(beforeStudyUID,afterStudyUID);
			}
			else {
				String afterStudyUID=studyUIDBeforeAndAfter.get(beforeStudyUID);
				dcmObj.remove(Tag.StudyInstanceUID);
				dcmObj.putString(Tag.StudyInstanceUID, VR.UI, afterStudyUID);
			}
			//修改SeriesUID
			String beforeSeriesUID=dcmObj.getString(Tag.SeriesInstanceUID);//get Study Instance UID
			if(beforeSeriesUID==null)
				beforeSeriesUID="1.2.3.4.5.6.7.8.9.0.1111";
			if(!serieUIDBeforeAndAfter.containsKey(beforeSeriesUID))
			{
				dcmObj.remove(Tag.SeriesInstanceUID);
				SimpleDateFormat dateFormat=new SimpleDateFormat("yyyyMMdd.HHmmss");
				
				String afterSeriesUID=String.format("%s.%s", beforeSeriesUID,dateFormat.format(new Date()));
				dcmObj.putString(Tag.SeriesInstanceUID, VR.UI, afterSeriesUID);
				serieUIDBeforeAndAfter.put(beforeSeriesUID, afterSeriesUID);
			}
			else {
				String afterSeriesUID=serieUIDBeforeAndAfter.get(beforeSeriesUID);
				dcmObj.remove(Tag.SeriesInstanceUID);
				dcmObj.putString(Tag.SeriesInstanceUID, VR.UI, afterSeriesUID);
			}
			//修改ImageUID和MediaStoreageSOPInstanceUID
			String beforeSOPInstanceUID=dcmObj.getString(Tag.SOPInstanceUID);
			if(beforeSOPInstanceUID==null)
				beforeSOPInstanceUID="1.2.3.4.5.6.7.8.9.0.2222";
			dcmObj.remove(Tag.SOPInstanceUID);
			dcmObj.putString(Tag.SOPInstanceUID, VR.UI, beforeSOPInstanceUID+".1");
			dcmObj.remove(Tag.MediaStorageSOPInstanceUID);
			dcmObj.putString(Tag.MediaStorageSOPInstanceUID, VR.UI,beforeSOPInstanceUID+".1");
			din.close();
			
			
			//Save Changes
			FileOutputStream fos;
			try {
				fos = new FileOutputStream(fileDCM);
				BufferedOutputStream bos=new BufferedOutputStream(fos);
				DicomOutputStream dos=new DicomOutputStream(bos);
				try {
					
					dos.writeDicomFile(dcmObj);
					dos.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		} catch (IOException e) {
			// TODO: handle exception
			e.printStackTrace();
			return;
		}

		
		
		
	}
}
