import java.util.List;
import java.util.Random;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class Proj3 {

	static int _pageSize = 20;
	static int _workingSetSize = 4;
	static int[] _callStack;
	static String _dataFileName = "data.dat";
	static String _pagingAlgorithm = "FIFO";
	static Boolean _suppressDebugOutput = false;
	static Page[] _pageSet = null;
	static int _callstackSize = 100;

	public static void main(String[] args) throws IOException {

		ParseCLIArgs(args);

		LoadDataIntoPages();

		println("Successfully loaded data into pageset");

		GenerateCallStack();

		println("Successfully generated callstack on data");
	}

	public static boolean GenerateCallStack() {

		_callStack = new int[_callstackSize];
		
		try {
			for (int i = 0; i < _callstackSize; i++) {
				_callStack[i] = new Random().nextInt(_pageSet.length);
			}
		} catch (Exception ex) {
			return false;
		}
		
		return true;

	}

	public static void LoadDataIntoPages() {
		try {

			BufferedReader in = new BufferedReader(
					new FileReader(_dataFileName));

			String str;
			while ((str = in.readLine()) != null) {

				int pageCount = str.length() / _pageSize + 1;

				_pageSet = new Page[pageCount];

				for (int i = 0; i < pageCount; i++) {
					Page page = new Page();

					if (str.length() <= (i) * _pageSize) {

						page.startIndex = i * _pageSize;
						page.endIndex = str.length() - 1;
						page.data = str.substring(page.startIndex)
								.toCharArray();

						break;
					}

					page.startIndex = i * _pageSize;
					page.endIndex = (i + 1) * _pageSize - 1;
					page.size = _pageSize;

					page.data = str.substring(page.startIndex, page.endIndex)
							.toCharArray();

					_pageSet[i] = page;
				}

				break;
			}
			in.close();

		} catch (IOException ex) {

		}

	}

	public static void ParseCLIArgs(String args[]) {

		try {
			while (true) {
				if (args.length <= 0)
					break;

				if (IsStringInteger(args[0])) {
					_pageSize = Integer.parseInt(args[0]);
				}

				if (args.length <= 1)

					break;

				if (IsStringInteger(args[1])) {
					_workingSetSize = Integer.parseInt(args[1]);
				}

				if (args.length <= 2)
					break;

				_dataFileName = args[2];

				break;

			}
		} catch (Exception e) {
			System.out.println("Invalid Arguments");
		}

		println("Using a page size of:" + _pageSize);
		println("Using a Working Set size of:" + _workingSetSize);
		println("Using input file:" + _dataFileName);
		println("Using paging schema: " + _pagingAlgorithm);
	}

	public static void println(String s) {

		println(s, true);
	}

	public static void println(String s, boolean isDebug) {

		if (!(isDebug && _suppressDebugOutput))
			System.out.println(s);

		return;
	}

	public static boolean IsStringInteger(String input) {
		try {
			Integer.parseInt(input);
			return true;
		} catch (Exception ex) {
			return false;
		}
	}

}
