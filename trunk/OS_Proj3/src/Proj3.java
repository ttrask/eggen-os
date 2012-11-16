import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.ArrayList;

public class Proj3 {

	static int _pageSize = 10;
	static int _workingSetSize = 3;
	static int[] _callStack;
	static String _dataFileName = "data.dat";
	static String _pagingAlgorithm = "clock";
	static String _callStackFileName = "callStack.dat";

	static Boolean _suppressDebugOutput = true;
	static Page[] _pageSet = null;
	static int _callstackSize = 500;

	public static void main(String[] args) throws IOException {

		// ParseCLIArgs(args);

		LoadDataIntoPages();

		println("Successfully loaded data into pageset");

		LoadCallStack();
	   
		//GenerateCallStack();

		println("Successfully generated callstack on data");

		ProcessWorkingSet("fifo");
		ProcessWorkingSet("lru");
		ProcessWorkingSet("optimal");
		ProcessWorkingSet("clock");
		
		System.exit(0);
	}

	public static void ProcessWorkingSet() {

		ProcessWorkingSet(_pagingAlgorithm);
	}

	public static void ProcessWorkingSet(String algorithm) {
		try {
			int pageFaults = 0;

			List<Page> workingSet = new LinkedList<Page>();

			for (int i = 0; i < _callStack.length; i++) {

				

				Page page = _pageSet[_callStack[i]];

				boolean pageFoundInWS = false;

				for (Page p : workingSet) {
					if (p.pageNumber == page.pageNumber) {
						// page is already in working set.
						// no page fault.

						p.LastAccessed = new Date();
						p.clockRef = 1;
						pageFoundInWS = true;
						break;
					}
				}

				// if the page isn't in the working set
				// try to add it
				if (!pageFoundInWS) {

					pageFaults++;

					if (workingSet.size() < _workingSetSize) {

						page.AddedToStack = new Date();
						page.LastAccessed = page.AddedToStack;
						page.clockRef = 1;

						workingSet.add(page);

					} else {

						Thread.sleep(100);
						// if the workign set is full adn the page isn't in it
						// yet
						// go and pull out a page and put a new one in.

						switch (algorithm.toLowerCase()) {

						case "fifo":
							ReplaceWorkingSetFIFO(workingSet, page);
							break;
						case "optimal":
							int[] futureStack = Arrays.copyOfRange(_callStack,
									i, _callStack.length);
							ReplaceWorkingSetOpt(workingSet, page, futureStack);
							break;
						case "clock":
							workingSet = ReplaceWorkingSetClock(workingSet,
									page);
							break;
						case "lru":
							ReplaceWorkingSetLRU(workingSet, page);
							break;
						}
					}

					String ws = "";

					for (Page p : workingSet) {
						ws += " " + p.pageNumber;
					}

					println(ws);
				}
			}

			println("Completed parsing working set.");
			
			println("Found " + pageFaults + " pagefaults using "+ algorithm, false);
		} catch (Exception ex) {

		}

		

	}

	protected static List<Page> ReplaceWorkingSetFIFO(List<Page> ws,
			Page newPage) {

		// instatiates a date to now
		Date earliestDate = new Date();
		Page pageToReplace = new Page();

		for (Page p : ws) {
			if (p.AddedToStack.getTime() < earliestDate.getTime()) {
				pageToReplace = p;
				earliestDate = p.AddedToStack;
			}
		}

		newPage.AddedToStack = new Date();
		newPage.LastAccessed = newPage.AddedToStack;

		ws.remove(pageToReplace);
		ws.add(newPage);

		return ws;
	}

	protected static boolean ReplaceWorkingSetOpt(List<Page> ws, Page newPage,
			int[] stack) {

		List<Integer> wsNumbers = new ArrayList<Integer>();

		int furthestPageAway = 0;

		for (Page p : ws) {
			wsNumbers.add(p.pageNumber);
		}

		for (int i = 0; i < stack.length; i++) {
			for (Integer pageNum : wsNumbers) {
				if (pageNum == stack[i]) {
					wsNumbers.remove(pageNum);
					break;
				}
			}

			if (wsNumbers.size() == 1) {
				furthestPageAway = wsNumbers.get(0);
				break;
			}

		}

		newPage.AddedToStack = new Date();
		newPage.LastAccessed = newPage.AddedToStack;

		for (Page p : ws) {
			if (p.pageNumber == furthestPageAway) {
				ws.remove(p);
				ws.add(newPage);
				break;

			}
		}

		return true;
	}

	protected static List<Page> ReplaceWorkingSetClock(List<Page> ws,
			Page newPage) {

		boolean foundVictim = false;

		for (int i = 0; i < ws.size(); i++) {

			Page p = ws.get(i);

			if (p.clockRef == 1) {

				// give pages a second chance.
				p.clockRef = 0;

			} else {

				foundVictim = true;

				// "move the clock" so to speak to the new page and put
				// everything after it.
				newPage.clockRef = 1;

				List<Page> newWs = new ArrayList<Page>();
				newWs.add(newPage);

				if (i < _workingSetSize - 1) {
					List<Page> pagesAfter = ws.subList(i + 1, ws.size());
					newWs.addAll(pagesAfter);
				}

				if (i > 0) {
					List<Page> pagesBefore = ws.subList(0, i);
					newWs.addAll(pagesBefore);
				}

				ws = newWs;

				break;
			}
		}

		if (!foundVictim) {
			return ReplaceWorkingSetClock(ws, newPage);
		}
		return ws;
	}

	protected static boolean ReplaceWorkingSetLRU(List<Page> ws, Page newPage) {

		Date earliestDate = new Date();
		Page pageToReplace = new Page();

		for (Page p : ws) {
			if (p.LastAccessed.getTime() < earliestDate.getTime()) {
				pageToReplace = p;
				earliestDate = p.LastAccessed;
			}
		}

		newPage.AddedToStack = new Date();
		newPage.LastAccessed = newPage.AddedToStack;

		ws.remove(pageToReplace);
		ws.add(newPage);

		return true;

	}

	public static boolean GenerateCallStack() {

		_callStack = new int[_callstackSize];

		String s = "";
		
		try {
			for (int i = 0; i < _callstackSize; i++) {
				_callStack[i] = new Random().nextInt(_pageSet.length);
				
				s+= " " + _callStack[i];
			}
		} catch (Exception ex) {
			return false;
		}

		println(s, false);
		
		return true;

	}

	public static void LoadCallStack() {
		try {

			BufferedReader in = new BufferedReader(new FileReader(
					_callStackFileName));

			String str;
			while ((str = in.readLine()) != null) {

				_callStack = new int[str.length()];
				List<Integer> stack = new ArrayList<Integer>();

				for (int i = 0; i < str.length(); i++) {

					_callStack[i] = Integer.parseInt(str.substring(i, i + 1));
				}

			}
			in.close();

		} catch (IOException ex) {

		}

	}

	public static void LoadDataIntoPages() {
		try {

			BufferedReader in = new BufferedReader(
					new FileReader(_dataFileName));

			String str;
			while ((str = in.readLine()) != null) {

				int pageCount = str.length() / _pageSize;

				_pageSet = new Page[pageCount];

				for (int i = 0; i < pageCount; i++) {
					Page page = new Page();

					page.pageNumber = i;

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

				if (args.length <= 3)
					break;

				_callStackFileName = args[3];

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
