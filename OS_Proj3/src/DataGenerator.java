import java.io.*;
import java.util.Random;


public class DataGenerator {
	
	private static int dataLength = 1000;
	
	
	public static void main(String[] args) throws IOException {

		String filename = "data.dat";

		// removes old file
		PrintStream out = new PrintStream(new FileOutputStream(filename));
		
		System.setOut(out);
		
		for(int i=0;i<dataLength;i++){
			
			int rand = new Random().nextInt(10);
			
			System.out.print(rand);
		}

	}
	
}
