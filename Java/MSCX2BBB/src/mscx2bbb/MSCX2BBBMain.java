package mscx2bbb;

public class MSCX2BBBMain {

	public static void main(String[] args) {
		boolean extra = false;
		if (args.length < 1)
		{
			System.out.println("[inputfilename -x(for extra information)");
			return;
		}
		if (args.length > 2)
		{
			System.out.println("too many arguments");
			return;
		}
		String filename = args[0].trim();
		
		if (args.length == 2)
		{
			extra = args[1].trim().equalsIgnoreCase("-x");
		}

		if (!filename.toLowerCase().endsWith(".mscx"))
		{
			System.out.println("Input file must be a *.mscx file");
			return;
		}
		
		MSCXImporter mscxConverter = new MSCXImporter(filename, extra);
		mscxConverter.Import();
		mscxConverter.Export();
		System.out.println("Done!");
		
	}

}
