package mscx2bbb;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.apache.commons.io.FilenameUtils;
import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import mscx2bbb.Token.Jump_Type;
import mscx2bbb.Token.Marker_Type;
import mscx2bbb.Token.Token_Type;

public class MSCXImporter implements ContentHandler {

	private String inputFilename = "";
	public String outputFilename = "";
	private String content = "";
	private String textStyle = "";
	private int staffId = 0;
	private int firstStaff = 0;
	private int bassStaff = 0;
	private int beatStaff = 0;
	private boolean inTieSpanner = false;
	private boolean isTieSpanned = false;
	private Token token;
	private int measureDuration = 0;
	private int measureTime = 0;
	private int voiceTime = 0;
	private int sigN = 0;
	private int sigD = 0;
	private int span = 0;
	private String subtype = "";
	private String spannerType = "";
	private String beginText = "";
	private String voltaText = "";
	private int normalNotes = 1;
	private int actualNotes = 1;
	private int spannerMeasures = 0;
	private boolean remeberEndRepeat = false;
	private String text = "";
	private String label = "";
	private String jumpTo = "";
	private String playUntil = "";
	public String continueAt = "";
	public boolean playRepeats = false;
	private boolean extra;
	//private int bassTranspose = 0;

	
	
	public List<Token> tokens = new ArrayList<Token>();
	public List<Token> exportTokens = new ArrayList<Token>();
	public List<Token> optimizedExportTokens = new ArrayList<Token>();
	



	public int tempo = 120;
	public String title = "";

	public MSCXImporter(String filename, boolean extra) {
		this.extra = extra;
		inputFilename = filename;
		outputFilename = FilenameUtils.getFullPath(filename) + FilenameUtils.getBaseName(filename).toUpperCase() + ".BBB";
	}
	


	public void Import() {
		try {
			// Create XMLReader
			SAXParserFactory parserFactory = SAXParserFactory.newInstance();
			SAXParser parser = parserFactory.newSAXParser();
			XMLReader xmlReader = parser.getXMLReader();

			// Set input file
			FileReader reader = new FileReader(inputFilename);
			InputSource inputSource = new InputSource(reader);

			xmlReader.setContentHandler(this);

			// parse it now
			xmlReader.parse(inputSource);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (ParserConfigurationException e) {
			e.printStackTrace();
		}

	}

	
	private void Optimize()
	{
		Token lastBbxLine = new Token(Token.Token_Type.NONE, 0);
		for (int i = 0; i<exportTokens.size() ; i++)
		{
			Token bbxLine = exportTokens.get(i);
			
			if (!bbxLine.bassLine.isBlank() || !bbxLine.beatLine.isBlank() 
					|| (i==0) 
					||  !bbxLine.jumpMarkLine.isBlank())
			{
				optimizedExportTokens.add(bbxLine);
				lastBbxLine = bbxLine;
			}
			else
			{
				// Limit duration to 64
				if (((bbxLine.time + lastBbxLine.time)< 64)
						&& lastBbxLine.jumpMarkLine.isBlank())
				{
					// add the new to the last
					optimizedExportTokens.get(optimizedExportTokens.size()-1).AddTime(bbxLine.time);
				}
				else
				{
					optimizedExportTokens.add(bbxLine);
					lastBbxLine = bbxLine;
				}
			}
		}
	}
	
	/**
	 * Export tokens with the same timestamp
	 * @param sameTimeTokens
	 */
	private void ExportTokens(List<Token> sameTimeTokens, int time)
	{
		int index=0;
		int timestamp;
		// Sort it, the first token must now be a bass token
		Collections.sort(sameTimeTokens);
		Token firstToken = sameTimeTokens.get(0);
		Token lastToken = sameTimeTokens.get(sameTimeTokens.size()-1);
		timestamp = firstToken.timestamp;
		
		// Check timestamp of all tokens which should have the same timestamp
		for (Token token : sameTimeTokens)
		{
			if (token.timestamp != timestamp)
			{
				System.err.println("Different timestamp");
			}
		}
		
		Token bbExportLine = new Token(firstToken);
		
		if (firstToken.isJump())
		{
			bbExportLine.jumpMarkLine = firstToken.getExportString(extra);

			if (sameTimeTokens.size()>2)
			{
				System.err.println("More than 2 jumps");
			}
			else if (sameTimeTokens.size()>1)
			{
				Token secondToken = sameTimeTokens.get(1);

				if (firstToken.jumpType == Token.Jump_Type.END_REPEAT 
						&& secondToken.jumpType == Token.Jump_Type.END_REPEAT_SWITCH)
				{
					bbExportLine.jumpMarkLine = secondToken.getExportString(extra);
				}
				else if (firstToken.jumpType == Token.Jump_Type.END_REPEAT_SWITCH 
						&& secondToken.jumpType == Token.Jump_Type.END_REPEAT)
				{
					bbExportLine.jumpMarkLine = firstToken.getExportString(extra);
				}
				else 
				{
					Token bbExportLineBefore = new Token(firstToken);
					bbExportLineBefore.jumpMarkLine = firstToken.getExportString(extra);

					exportTokens.add(bbExportLineBefore);
					
					bbExportLine = new Token(secondToken);
					bbExportLine.jumpMarkLine = secondToken.getExportString(extra);
				}
			}
		}
		else if (firstToken.isMarker())
		{
			bbExportLine.jumpMarkLine = firstToken.getExportString(extra);
			if (sameTimeTokens.size()>2)
			{
				System.err.println("More than 2 markers");
			}
		}
		else if (firstToken.isTempo())
		{
			bbExportLine.jumpMarkLine = lastToken.getExportString(extra);
			if (sameTimeTokens.size()>2)
			{
				System.err.println("More than 2 tempo markers");
			}
		}
		else
		{
			Token bassToken = firstToken;
			Token beatToken  = new Token(Token.Token_Type.BEAT);
			
			// Print time
			if ((time % 8) != 0)
			{
				System.err.println("Duration must be 1/16 or longer");
			}
			bbExportLine.time = time / 8;

			if (bassToken.isBass())
			{
				index=1;
			}
			else
			{
				bassToken = new Token(Token.Token_Type.BASS);
			}
			bbExportLine.bassLine = bassToken.getExportString(extra);
			bbExportLine.tokenType = Token_Type.BASS;
			
			// Is there no beat, export an empty string?
			for (;index<sameTimeTokens.size(); index++)
			{
				beatToken.Merge(sameTimeTokens.get(index));
			}
			bbExportLine.beatLine = beatToken.getExportString(extra);
			bbExportLine.tokenType = Token_Type.BEAT;
		}
		exportTokens.add(bbExportLine);
	}
	

	/**
	 * Find the marker 
	 * @param marker
	 * @return
	 */
	private int GetChordAfterMarker(String marker)
	{
		if (marker.isBlank())
		{
			return -1;
		}
		
		for (int i=0; i< optimizedExportTokens.size(); i++)
		{
			Token bbxline = optimizedExportTokens.get(i);
			if (bbxline.isMarker())
			{
				if (bbxline.marker.equals(marker))
				{
					return GetNextBassBeat(i);
				}
			}
		}
		if (marker.equalsIgnoreCase("start"))
		{
			return 0;
		}
		System.err.println("No marker \"" + marker + "\" found");
		return -1;
	}
	
	
	
	
	/**
	 * Find the chord at a specified timestamp
	 * @param timestamp
	 * @return
	 */
	private int GetChordAtTime(int timestamp)
	{
		for (int i=0; i< optimizedExportTokens.size(); i++)
		{
			Token bbxline = optimizedExportTokens.get(i);
			if (bbxline.isChord())
			{
				if (bbxline.timestamp == timestamp)
				{
					return i;
				}
			}
		}
		System.err.println("No Chord at timestamp " + timestamp + " found");
		return -1;
	}
	
	

	/**
	 * Find the start repeat token before the end repeat token
	 * @param index
	 * @return
	 */
	private int GetNextBassBeat(int index)
	{
		for (int i=index; i< optimizedExportTokens.size(); i++)
		{
			Token bbxline = optimizedExportTokens.get(i);
			if (bbxline.isChord())
			{
				return i;
			}
		}
		System.err.println("No Chord after index " + index + " found");
		return -1;
	}
	
	/**
	 * Find the start repeat token before the end repeat token
	 * @param index
	 * @return
	 */
	private int GetStartRepeatBefore(int index)
	{
		for (int i=index; i>=0; i--)
		{
			Token bbxline = optimizedExportTokens.get(i);
			if (bbxline.isMarker())
			{
				if (bbxline.markerType == Marker_Type.START_REPEAT)
				{
					return GetNextBassBeat(i);
				}
			}
		}
		return 0;
	}
	
	
	/**
	 * Returns the line number of an optimizedExportTokens 
	 * 
	 * @param index of the optimizedExportTokens
	 * @return lineNr or -1 if not found
	 */
	private int getLineNrOfOptExpToken(int index)
	{
		if (index == -1)
		{
			return -1;
		}
		if (index >= optimizedExportTokens.size())
		{
			return -1;
		}
		return optimizedExportTokens.get(index).lineNr;
	}
	
	private void RemoveToken(int index)
	{
		optimizedExportTokens.remove(index);
		for (int i=index; i< optimizedExportTokens.size() ; i++)
		{
			Token token = optimizedExportTokens.get(i);
			token.lineNr --;
		}
		for (int i=0; i< optimizedExportTokens.size() ; i++)
		{
			Token token = optimizedExportTokens.get(i);
			if (token.tokenType == Token_Type.JUMP)
			{
				if (token.jumpToLine > index) token.jumpToLine--;
				if (token.playUntilLine > index) token.playUntilLine--;
				if (token.continueAtLine > index) token.continueAtLine--;
				// Generate new line
				token.jumpMarkLine = token.getExportString(extra);
			}
		}
	}
	
	/**
	 * Check one repetition block
	 */
	private boolean CheckSingleRepetition(int pos, int amountBack, int number)
	{
		int start = pos-amountBack+1;
		for (int i=start; i<=pos ; i++)
		{
			int first = i - (number*amountBack);
			if (first < 0) return false;
			if (!optimizedExportTokens.get(first).sameChord(optimizedExportTokens.get(i)))
				return false;
		}
		return true;
	}

	/**
	 * Check for repetition and replace it
	 */
	private boolean ReplaceRepetition(int pos, int amountBack)
	{
		
		int blocks = 0;
		while (CheckSingleRepetition(pos, amountBack, blocks + 1))
		{
			blocks++;
		}
		if (blocks == 0) return false;
		
		//System.out.println(pos + " " + amountBack + " " + blocks);
		
		// A loop with 1 run has no advantage
		if (amountBack<=1 && blocks<=1) return false;
		
		int start = pos - blocks*amountBack + 1;
		optimizedExportTokens.get(start).MakeRelRepeat(amountBack, blocks+1, extra);
		
		for (int i=1; i<(amountBack * blocks); i++)
		{
			RemoveToken(start+1);
		}
		return true;
	}
	
	/**
	 * Reduce the lines
	 */
	private void Compress()
	{
		boolean found ;
		
		do {
			found = false;
			for (int ii=1; ii < optimizedExportTokens.size() && !found; ii++)
			{
				for (int i=optimizedExportTokens.size()-1; i>0  && !found; i--)
				{
					if (ReplaceRepetition(i,ii))
					{
						found = true;
					}
				}
			}
		} while (found);
	}
	
	/**
	 * Remove unused markers
	 */
	private void RemoveUnusedMarkers()
	{
		
		for (int i=optimizedExportTokens.size()-1; i>=0 ; i--)
		{
			Token token = optimizedExportTokens.get(i);
			if (token.tokenType == Token_Type.MARK &&
					token.lineNr == -1)
			{
				optimizedExportTokens.remove(i);
			}
		}
	}
	
	/**
	 * Fill the jumps with line numbers as destinations
	 */
	private void JumpDestinations()
	{
		for (int i=0; i< optimizedExportTokens.size(); i++)
		{
			Token bbxline = optimizedExportTokens.get(i);

			int timestamp = bbxline.timestamp;
			int voltaMeasures = bbxline.voltaMeasures;
			if (bbxline.tokenType == Token_Type.JUMP)
			{
				if ((bbxline.jumpType == Jump_Type.END_REPEAT_SWITCH) 
						|| (bbxline.jumpType == Jump_Type.END_REPEAT))
				{
					bbxline.jumpToLine = 
							getLineNrOfOptExpToken(GetStartRepeatBefore(i));
				}
				if (bbxline.jumpType == Jump_Type.VOLTA)
				{
					bbxline.jumpToLine = 
							getLineNrOfOptExpToken(GetChordAtTime(timestamp + voltaMeasures * measureDuration));
				}
				if (bbxline.jumpType == Jump_Type.JUMP)
				{
					bbxline.jumpToLine = 
							getLineNrOfOptExpToken(GetChordAfterMarker(bbxline.jumpTo));
					bbxline.playUntilLine = 
							getLineNrOfOptExpToken(GetChordAfterMarker(bbxline.playUntil));
					bbxline.continueAtLine = 
							getLineNrOfOptExpToken(GetChordAfterMarker(bbxline.continueAt));
					
				}
				
				// Generate new line
				bbxline.jumpMarkLine = bbxline.getExportString(extra);
				
				// Mark the destination as jump desination
				if (bbxline.jumpToLine >= 0)
				{
					for (int ii=0; ii< optimizedExportTokens.size(); ii++)
					{
						if (bbxline.jumpToLine == optimizedExportTokens.get(ii).lineNr)
						{
							optimizedExportTokens.get(ii).isJumpDestination = true;
						}
					}
				}
			}
		}
		
	}
	
	/**
	 * Fill the tokens with line numbers
	 */
	private void LineNumbers()
	{
		int lineNr=0;
		for (Token bbxline : optimizedExportTokens)
		{
			if (bbxline.isBass() || bbxline.isBeat() || bbxline.isRest()
					|| bbxline.isJump()
					|| bbxline.isTempo()
					|| (bbxline.isMarker() && (bbxline.markerType == Marker_Type.END))) 
			{
				bbxline.lineNr = lineNr;
				lineNr ++;
			}
		}
	}
	
	
	/**
	 * Export all Tokens
	 */
	public void Export() {
		Collections.sort(tokens);

		
		int lastTimestamp = 0;
		List<Token> sameTimeTokens = new ArrayList<Token>();
		boolean wasJump=false;
		boolean wasMarker=false;
		boolean wasTempo=false;
		for (int i = 0; i < tokens.size(); i++) {
			//System.out.println(i);
			Token thisToken = tokens.get(i);
			
			if ((thisToken.timestamp != lastTimestamp)
					|| (thisToken.isJump() != wasJump)
					|| (thisToken.isMarker() != wasMarker)
					|| (thisToken.isTempo() != wasTempo)
					)
			{
				if (sameTimeTokens.isEmpty())
				{
					if (i>0)
					{
						System.err.println("Empty token collection");
					}
				}
				else
				{
					ExportTokens (sameTimeTokens, thisToken.timestamp - lastTimestamp);
					sameTimeTokens = new ArrayList<Token>();
					lastTimestamp = thisToken.timestamp;
				}
			}
			sameTimeTokens.add(thisToken);
			wasJump = thisToken.isJump();
			wasMarker = thisToken.isMarker();
			wasTempo = thisToken.isTempo();
		}

		if (!sameTimeTokens.isEmpty())
		{
			ExportTokens ( sameTimeTokens, 0);
		}

		Optimize();
		LineNumbers();
		JumpDestinations();
		RemoveUnusedMarkers();
		Compress();
		
		PrintWriter pw;
		try {
			pw = new PrintWriter(outputFilename);
			pw.println("BBB:1");
			pw.println("TITLE:"+title);
			//pw.println("TEMPO:" + String.format("%03d",tempo));
			pw.println("START");
			int lineNr = 0;
			for (Token bbxline : optimizedExportTokens)
			{
				if (!bbxline.isMarker())
				{
					if (bbxline.lineNr>=0)
					{
						pw.print(String.format("%3d", bbxline.lineNr));
						
						// Check line number
						if (!extra)
						{
							if (lineNr != bbxline.lineNr)
							{
								System.err.println("Problems with line numbers");
							}
						}
					}
					else
					{
						pw.print("   ");
					}
					pw.print(":");
					if (bbxline.isJumpDestination)
					{
						pw.print("*");
					}
					else
					{
						pw.print(" ");
					}
					
					if (extra)
					{
						pw.print(String.format("%5d", bbxline.timestamp));
						pw.print("(");
						pw.print(String.format("%3d", ((bbxline.timestamp / measureDuration)+1)));
						pw.print(".");
						pw.print(String.format("%02d", ((bbxline.timestamp % measureDuration)/8)));
						pw.print(")");
						pw.print(":");
					}
					if (bbxline.jumpMarkLine.isBlank())
					{
						pw.print("BB:");
						pw.print(String.format("%02d", bbxline.time));
						pw.print(":");
						pw.print(bbxline.bassLine);
						pw.print(":");
						pw.print(bbxline.beatLine);
						pw.println();
					}
					else
					{
						if (bbxline.time != 0)
						{
							System.err.println("Marker or jump with duration != 0");
						}
						pw.println(bbxline.jumpMarkLine);
					}
					lineNr++;
				}
				else
				{
					// it was a marker
					pw.println(bbxline.jumpMarkLine);
				}
			}
			pw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void setDocumentLocator(Locator locator) {
	}

	@Override
	public void startDocument() throws SAXException {
	}

	@Override
	public void endDocument() throws SAXException {
	}

	@Override
	public void startPrefixMapping(String prefix, String uri) throws SAXException {
	}

	@Override
	public void endPrefixMapping(String prefix) throws SAXException {
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException {
		content = "";
		
		if (qName.equals("endTuplet")) {
			normalNotes = 1;
			actualNotes = 1;
		}

		if (qName.equals("Jump")) {
			jumpTo = "";
			playUntil = "";
			continueAt = "";
			playRepeats = false;
		}
		if (qName.equals("Marker")) {
			text = "";
			label = "";
		}
		if (qName.equals("BarLine")) {
			span = 0;
			subtype = "";
		}
		if (qName.equals("Volta")) {
			beginText = "";
		}
		
		if (qName.equals("Text")) {
			textStyle = "";
		}
		if (qName.equals("Staff")) {
			staffId = Integer.parseInt(atts.getValue("id"));
			measureTime = 0;
			voiceTime = 0;
		}
		if (qName.equals("Instrument")) {
			if (atts.getValue("id").equals("acoustic-bass"))
			{
				bassStaff = staffId;
				if (firstStaff == 0 || bassStaff < firstStaff)
				{
					firstStaff = bassStaff;
				}
			}
			if (atts.getValue("id").equals("drumset"))
			{
				beatStaff = staffId;
				if (firstStaff == 0 || beatStaff < firstStaff)
				{
					firstStaff = beatStaff;
				}
			}
		}
		if (qName.equals("voice")) {
			voiceTime = 0;
		}
		if (qName.equals("Chord")) {
			token = new Token(Token.Token_Type.NONE);
		}
		if (qName.equals("Rest")) {
			token = new Token(Token.Token_Type.REST);
		}
		if (qName.equals("Spanner")) {
			spannerMeasures = 0;
			voltaText = "";
			inTieSpanner = false;
			spannerType = atts.getValue("type");
			if (spannerType == null)
			{
				spannerType ="";
			}
			else if (spannerType.equals("Tie")) 
			{
				inTieSpanner = true;
			}
		}
		if (qName.equals("prev")) {
			if (inTieSpanner)
			{
				isTieSpanned = true;
			}
		}



	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		if (qName.equals("transposeChromatic")) {
			if (staffId == bassStaff)
			{
				//bassTranspose = Integer.parseInt(content);
			}
		}
		
		if (qName.equals("normalNotes")) {
			normalNotes = Integer.parseInt(content);
		}
		if (qName.equals("actualNotes")) {
			actualNotes = Integer.parseInt(content);
		}
		if (qName.equals("normalNotes")) {
			normalNotes = Integer.parseInt(content);
		}
		if (qName.equals("tempo")) {
			tempo = (int) (Double.parseDouble(content) * 60.0);
			Token tempoToken = new Token(Token.Token_Type.TEMPO, measureTime );
			tempoToken.tempo = tempo;
			tokens.add(tempoToken);
		}
		if (qName.equals("Jump")) {
			tokens.add(
					new Token(jumpTo, playUntil, continueAt, playRepeats,
							measureTime + measureDuration ));
		}
		if (qName.equals("jumpTo")) {
			jumpTo = content.trim();
		}
		if (qName.equals("playUntil")) {
			playUntil = content.trim();
		}
		if (qName.equals("continueAt")) {
			continueAt = content.trim();
		}
		if (qName.equals("playRepeats")) {
			playRepeats = Integer.parseInt(content) > 0;
		}

		if (qName.equals("Text")) {
			if (textStyle.equals("Title"))
			title = content.trim();
		}
		if (qName.equals("style")) {
			textStyle = content.trim();
		}
		if (qName.equals("measures")) {
			spannerMeasures = Integer.parseInt(content);
		}
		if (qName.equals("beginText")) {
			beginText = content.trim();
		}
		if (qName.equals("text")) {
			text = content.trim();
		}
		if (qName.equals("label")) {
			label = content.trim();
		}
		if (qName.equals("Marker")) {

			
			// If it's "To Coda", add a jump to coda
			if (text.equalsIgnoreCase("To Coda"))
			{
				tokens.add(
						new Token(Token.Jump_Type.JUMP,
								measureTime, label));
			}
			// If it's "Fine", add also a jump to end
			else if (text.equalsIgnoreCase("Fine"))
			{
				tokens.add(
						new Token(Token.Jump_Type.FINE,
								measureTime + measureDuration ));
			}
			else
			{
				// Add a marker
				tokens.add(
						new Token(label,
								measureTime));
			}
		}
		
		if (qName.equals("Volta")) {
			voltaText = beginText;
		}

		if (qName.equals("startRepeat") && staffId == firstStaff) {
			tokens.add(
					new Token("[",
							measureTime));
		}
		if (qName.equals("endRepeat") && staffId == firstStaff) {
			remeberEndRepeat = true;
		}
		
		if (qName.equals("subtype")) {
			subtype = content.trim();
		}
		
		if (qName.equals("Part")) {
			staffId = 0;
		}
		if (qName.equals("Measure")) {
			if (measureDuration == 0)
			{
				System.err.println("Measure duration not set!");
			}
			measureTime += measureDuration;
			voiceTime = 0;
			
			if (remeberEndRepeat)
			{
				remeberEndRepeat = false;
				tokens.add(
						new Token(Token.Jump_Type.END_REPEAT,
								measureTime));
			}
		}
		if (qName.equals("BarLine") && staffId == firstStaff) {
			if (subtype.equals("end-repeat"))
			{
				if (span == 1)
				{
					tokens.add(
							new Token(Token.Jump_Type.END_REPEAT_SWITCH,
									measureTime + voiceTime));
				}
				else
				{
					tokens.add(
							new Token(Token.Jump_Type.END_REPEAT,
									measureTime + voiceTime));
				}
			}
		}
		if (qName.equals("span")) {
			span = Integer.parseInt(content);
		}
		if (qName.equals("sigN")) {
			sigN = Integer.parseInt(content);
		}
		if (qName.equals("sigD")) {
			sigD = Integer.parseInt(content);
		}
		if (qName.equals("TimeSig")) {
			if (sigN != 0 && sigD != 0)
			{
				int duration = sigN * 128 / sigD * 3;
				if (measureDuration != 0 && measureDuration !=duration)
				{
					System.err.println("Different time measure duration");
				}
				measureDuration = duration;
			}
			else
			{
				System.err.println("Invalid time signature");
			}
		}
		

		if (qName.equals("durationType")) {
			int dur = 1;
			if (content.equals("128th")) {
				dur = 128;
			} else if (content.equals("64th")) {
				dur = 64;
			} else if (content.equals("32nd")) {
				dur = 32;
			} else if (content.equals("16th")) {
				dur = 16;
			} else if (content.equals("eighth")) {
				dur = 8;
			} else if (content.equals("quarter")) {
				dur = 4;
			} else if (content.equals("half")) {
				dur = 2;
			} else if (content.equals("whole")) {
				dur = 1;
			} else if (content.equals("measure")) {
				dur = 1;
			} else {
				System.err.println("Unknown Duration: " + content);
			}

			token.setDuration(128 / dur, normalNotes, actualNotes);
		}

		if (qName.equals("duration")) {
			if (content.equals("4/4")) {
				token.setDuration(128, normalNotes, actualNotes);
			} else {
				System.err.println("Unknown Duration: " + content);
			}
		}
		
		if (qName.equals("subtype")) {
			if (content.contains("articAccent")) {
				token.isArticulated = true;
			}
		}

		
		if (qName.equals("Spanner") && staffId == firstStaff) {
			inTieSpanner = false;
			if (voltaText.equals("1."))
			{
				Token jump = new Token(Token.Jump_Type.VOLTA,
						measureTime + voiceTime);
				jump.voltaMeasures = spannerMeasures;
				tokens.add(jump);
			}
		}
		
		if (qName.equals("pitch")) {
			int note = Integer.parseInt(content);
			//note += bassTranspose;
			if (isTieSpanned)
			{
				token.tokenType = Token.Token_Type.REST;
			}
			else
			{
				token.notes.add(note);
			}
		}

		if (qName.equals("Chord") || qName.equals("Rest")) {
			if (staffId > 0)
			{
				token.timestamp = measureTime + voiceTime;
				if (staffId == bassStaff)
				{
					token.tokenType = Token.Token_Type.BASS;
					tokens.add(token);
				}
				if (staffId == beatStaff)
				{
					token.tokenType = Token.Token_Type.BEAT;
					tokens.add(token);
				}
			}
			voiceTime += token.getDuration();
			
			isTieSpanned = false;
		}
		if (qName.equals("dots")) {
			token.hasDot = true;
		}
		if (qName.equals("Score")) {
			Token token = new Token(Token_Type.MARK, measureTime);
			token.markerType = Marker_Type.END;
			tokens.add(token);
		}
	}

	@Override
	public void characters(char[] ch, int start, int length) throws SAXException {
		for (int i = start; i < (start + length); i++) {
			content += ch[i];
		}
	}

	@Override
	public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException {
	}

	@Override
	public void processingInstruction(String target, String data) throws SAXException {
	}

	@Override
	public void skippedEntity(String name) throws SAXException {
	}

}
