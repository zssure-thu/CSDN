package changeUID4Dcm.utils;

import java.util.ArrayList;

public class test {

	public test() {
		// TODO Auto-generated constructor stub
	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		AutoChangeDicomUID autoUID=new AutoChangeDicomUID();
		autoUID.scanDirectory("D:\\Documents\\Downloads\\WANGYUNQIANGDICOM");
		autoUID.changeAllUIDs();
		
	}

}
