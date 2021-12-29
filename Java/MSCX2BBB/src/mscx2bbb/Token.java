package mscx2bbb;

import java.util.ArrayList;
import java.util.List;

public class Token implements Comparable<Token> {

	public enum Token_Type
	{
	  NONE, JUMP, MARK, BASS, BEAT, REST, TEMPO
	}
	public enum Marker_Type
	{
	  NONE, START_REPEAT, MARKER, END
	}
	public enum Jump_Type
	{
	  NONE, END_REPEAT, END_REPEAT_SWITCH, REPEAT_REL, VOLTA, FINE, JUMP
	}
	public int timestamp;
	private int duration;
	public Boolean hasDot = false;
	public Boolean isArticulated = false;
	public Marker_Type markerType = Marker_Type.NONE;
	public Jump_Type jumpType = Jump_Type.NONE;
	public Token_Type tokenType = Token_Type.NONE;
	
	public List<Integer> notes = new ArrayList<Integer>();

	public int time = 0;
	public String bassLine = "";
	public String beatLine = "";
	public String jumpMarkLine = "";
	public int voltaMeasures = 0;
	
	public String jumpTo = "";
	public String playUntil = "";
	public String continueAt = "";
	public boolean playRepeats = false;
	public boolean toFine = false;
	
	public String marker = "";
	public int lineNr = -1;
	
	public int jumpToLine = -1;
	public int playUntilLine = -1;
	public int continueAtLine = -1;
	public boolean isJumpDestination = false;
	public int repeatLinesBack = 0;
	public int repeatCnt = 0;
	
	
	public int tempo = 0;
	
	public Token()
	{
	}
	public Token(Token other)
	{
		timestamp = other.timestamp;
		
		tokenType = other.tokenType;
		jumpType = other.jumpType;
		markerType = other.markerType;
		marker = other.marker;
		isArticulated = other.isArticulated;
		voltaMeasures = other.voltaMeasures;
		jumpTo = other.jumpTo;
		playUntil = other.playUntil;
		continueAt = other.continueAt;
		playRepeats = other.playRepeats;
		toFine = other.toFine;

	}
	
	public Token(Token_Type tokenType)
	{
		this.tokenType = tokenType;
	}
	public Token(Token_Type tokenType, int timestamp)
	{
		this.tokenType = tokenType;
		this.timestamp = timestamp;
	}
	
	public Token(Jump_Type jumpType, int timestamp)
	{
		this.tokenType = Token_Type.JUMP;
		this.jumpType = jumpType;
		this.timestamp = timestamp;
	}
	public Token(Jump_Type jumpType, int timestamp, String jumpTo)
	{
		this.tokenType = Token_Type.JUMP;
		this.jumpType = jumpType;
		this.timestamp = timestamp;
		this.jumpTo = jumpTo;
	}
	
	public Token(String marker, int timestamp)
	{
		this.tokenType = Token_Type.MARK;
		if (marker.equals("["))
		{
			this.markerType = Marker_Type.START_REPEAT;
		}
		else
		{
			this.markerType = Marker_Type.MARKER;
		}
		this.marker = marker;
		this.timestamp = timestamp;
		if (marker.isBlank())
		{
			System.err.println("Blank marker");
		}
	}

	public Token(String jumpTo, String playUntil, String continueAt, boolean playRepeats, int timestamp)
	{
		this.tokenType = Token_Type.JUMP;
		this.jumpType = Jump_Type.JUMP;
		this.jumpTo = jumpTo;
		this.playUntil = playUntil;
		this.continueAt = continueAt;
		this.playRepeats = playRepeats;
		this.timestamp = timestamp;
		
		// DC al fine?
		if (playUntil.equalsIgnoreCase("fine") && continueAt.isBlank())
		{
			this.toFine = true;
			this.playUntil = "";
		}

	}
	
	public void setDuration(int value, int normalNotes, int actualNotes)
	{
		duration = value * 3 * normalNotes;
		
		if (actualNotes == 0)
		{
			System.err.println("actualNotes has value 0");
			return;
		}
		if ((duration % actualNotes) != 0)
		{
			System.err.println("Invalid actualNotes:" + actualNotes);
			return;
		}
		duration/= actualNotes;
	}
	
	public int getDuration() {
		if (hasDot) {
			return (duration + duration / 2);
		} else {
			return duration;
		}
	}
	
	public boolean isJump()
	{
		return tokenType == Token_Type.JUMP;
	}
	public boolean isMarker()
	{
		return tokenType == Token_Type.MARK;
	}
	public boolean isRest()
	{
		return tokenType == Token_Type.REST;
	}
	public boolean isBass()
	{
		return tokenType == Token_Type.BASS;
	}
	public boolean isBeat()
	{
		return tokenType == Token_Type.BEAT;
	}
	public boolean isChord()
	{
		return isBass() || isBeat() || isRest();
	}
	public boolean isRepeat_Rel()
	{
		return isJump() && jumpType == Jump_Type.REPEAT_REL;
	}
	public boolean isRepeatableChord()
	{
		return isBass() || isBeat() || isRest() || isRepeat_Rel();
	}
	public boolean isTempo()
	{
		return tokenType == Token_Type.TEMPO;
	}

	/**
	 * Merge 2 beat tokens from 2 voices
	 * @param other
	 */
	public void Merge(Token other)
	{
		if (!this.isBeat() || !other.isBeat())
		{
			System.err.println("Only beats can have more than 1 voices.");
			return;
		}
		for(Integer otherNote : other.notes)
		{
			if (this.notes.contains(otherNote))
			{
				System.err.println("Note already in other voice");
				return;
			}
			this.notes.add(otherNote);
		}
	}
	
	/**
	 * Returns the cord to export it
	 * @return
	 */
	public String getExportString(boolean extra)
	{
		String ret = "";
		if (isBass()) {
			ret = "   ";
			if (notes.size() == 1)
			{
				int note = notes.get(0);
				
				/*
				note = note - 24;
				if (note < 0 || note >= 32)
				{
					System.err.println("Bass pitch out of range");
				}
				*/
				
				ret = String.format("%02d", note);
				if (isArticulated)
				{
					ret += ">";
				}
				else
				{
					ret += " ";
				}
			}
			else if (notes.size() == 0)
			{
				// nothing
			}
			else if (notes.size() > 1)
			{
				System.err.println("Error: bass must not have more than 1 note per token");
			} 
			
		} else if (isBeat()) {
			ret = "          ";
			StringBuilder field = new StringBuilder(ret);
			
			for (int note : notes)
			{
				if (note == 30) field.setCharAt(0, 'X');
				if (note == 32) field.setCharAt(1, 'x');
				if (note == 34) field.setCharAt(2, '.');
				if (note == 36) field.setCharAt(3, '*');
				if (note == 38) field.setCharAt(4, 'o');
				if (note == 40) field.setCharAt(5, 'T');
				if (note == 42) field.setCharAt(6, 'I');
				if (note == 44) field.setCharAt(7, '#');
				if (note == 49) field.setCharAt(8, '+');
				if (note == 50) field.setCharAt(9, 'v');
			}
			ret = field.toString();
			
		} else if (isJump()) {
			String jumpToLineStr = "   ";
			String playUntilLineStr = "   ";
			String continueAtLineStr = "   ";
			if (jumpToLine >=0)
			{
				jumpToLineStr = String.format("%03d", jumpToLine);
			}
			
			if (playUntilLine >=0)
			{
				playUntilLineStr = String.format("%03d", playUntilLine);
			}
			
			if (continueAtLine >=0)
			{
				continueAtLineStr = String.format("%03d", continueAtLine);
			}
			
			if (time != 0)
			{
				System.err.println("Jump duration must be 0");
			}

			if (jumpType == Jump_Type.END_REPEAT)
			{
				ret = "] :" + jumpToLineStr;
			}
			else if (jumpType == Jump_Type.END_REPEAT_SWITCH)
			{
				ret = "]S:"+ jumpToLineStr;
			}
			else if (jumpType == Jump_Type.REPEAT_REL)
			{
				ret = "< :"+ String.format("%03d", repeatLinesBack) +
						":" + String.format("%03d", repeatCnt);
			}
			else if (jumpType == Jump_Type.VOLTA)
			{
				ret = "V1:" + jumpToLineStr;
				if (extra)
				{
					ret += "(" + voltaMeasures + ")";
				}
			}
			else if (jumpType == Jump_Type.FINE)
			{
				ret = "FI";
			}
			else if (jumpType == Jump_Type.JUMP)
			{
				ret = "J" + (playRepeats?"R":"P") + (toFine?"F":" ")+ ":" + jumpToLineStr + ">" + playUntilLineStr + ">" + continueAtLineStr ;
				if (extra)
				{
					ret += "(" +jumpTo + ">" + playUntil + ">" + continueAt + ")";
				}
			}
			else
			{
				System.err.println("Undefined jump");
			}
		} else if (isMarker()) {
			if (time != 0)
			{
				System.err.println("Maker duration must be 0");
			}
			if (markerType == Marker_Type.START_REPEAT)
			{
				ret = "[ ";
			}
			else if (markerType == Marker_Type.END)
			{
				ret = "END";
			}
			else if (markerType == Marker_Type.MARKER)
			{
				ret = "MA" + ":" + marker;
			}
		} else if (isTempo())
		{
			ret = "TP:"+String.format("%03d", tempo);
		} else {
			System.err.println("Token is neither bass nor beat or jump or marker");
		}
		return ret;
	}
	
	public void AddTime(int timediff)
	{
		time += timediff;
	}
	
	private int sortOrder()
	{
		if (this.tokenType == Token_Type.JUMP)
		{
			if (jumpType == Jump_Type.VOLTA || jumpType == Jump_Type.FINE)
			{
				// Place it after the MARK
				return 2;
			}
			else
			{
				return 0;
			}
		}
		if (this.tokenType == Token_Type.MARK) return 1;
		if (this.tokenType == Token_Type.BASS) return 3;
		if (this.tokenType == Token_Type.BEAT) return 4;
		if (this.tokenType == Token_Type.REST) return 5;
		
			
		return 0;
	}
	
	public void MakeRelRepeat(int amountBack, int blocks, boolean extra)
	{
		tokenType = Token_Type.JUMP;
		jumpType = Jump_Type.REPEAT_REL;
		time = 0;
		repeatLinesBack = amountBack;
		repeatCnt = blocks;
		jumpMarkLine = getExportString(extra);
	}

	@Override
	public int compareTo(Token o) {
		if (this.timestamp < o.timestamp) {
			return -1;
		}
		if (this.timestamp > o.timestamp) {
			return +1;
		}
		return Integer.compare(this.sortOrder(), o.sortOrder());
	}
	
	public boolean sameChord(Token o)
	{
		if (!this.isRepeatableChord()) return false;
		if (!o.isRepeatableChord()) return false;
		if (this.isRepeat_Rel() != o.isRepeat_Rel()) return false;
			
		if (isRepeat_Rel())
		{
			if (this.repeatLinesBack != o.repeatLinesBack ) return false;
			if (this.repeatCnt != o.repeatCnt ) return false;
		}
		else
		{
			if (this.duration != o.duration) return false;
			if (this.isArticulated != o.isArticulated) return false;
			if (this.hasDot != o.hasDot) return false;
			if (this.time != o.time) return false;
			if (!this.bassLine.equals(o.bassLine)) return false;
			if (!this.beatLine.equals(o.beatLine)) return false;
		}
		return true;
	}
	
	@Override
	public String toString() {
		return "tokenType=" + tokenType + ", jumpType=" + jumpType + ", markerType=" + markerType + "\n" + 
				"timestamp=" + timestamp + ", duration=" + duration + "\n" +
				"jumpToLine=" + jumpToLine + " isJumpDestination=" + isJumpDestination + "\n";
		
		
				
	}

	

}
