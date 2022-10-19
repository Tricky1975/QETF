#include <ArgParse.h>
#include <QCol.hpp>
#include <Hex.hpp>
#include <QuickString.hpp>
#include <QuickStream.hpp>

using namespace std;
using namespace TrickyUnits;

int Errors{ 0 };
void Error(string E) {	
	QCol->Red("Error #" + to_string(++Errors) + ": ");
	QCol->Yellow(E + "\n");
}
#define OError(E) { Error(E); return;}
#define CError(E) { BT->Close(); OError(E) }

inline string Tabs(byte n) {
	string ret{ "" };
	for (byte i = 0; i < n; i++) ret += "\t";
	return ret;
}

void Process(string ifile, string ofile, string onamespace, string ovar,bool cpp, bool head, bool nor) {
	string defi{};
	string txt{"\""};
	byte tabs{ 1 };
	if (!ovar.size()) ovar = "StringVar";
	if (onamespace.size()) tabs++;
	if (!FileExists(ifile)) OError("File "+ifile+" not found!");
	QCol->Doing("Processing", ifile);
	//if (!head) defi = "static "; 
	if (cpp) defi += "string " + ovar; else defi += "char " + ovar + "[" + to_string((int)FileSize(ifile) + 2) + "]";
	auto BT{ ReadFile(ifile) };
	for (size_t pos = 0; pos < BT->Size(); pos++) {
		auto b{ BT->ReadByte() };
		if (b > 127) CError("Non-ASCII character found. Source is binary?");
		switch (b){
		case 0: CError("Null character found. C expects 0 for termination, but this is too eary. Source is binary?");
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 12:
			CError("Character found not common in strings. Source is binary?");
		case 9:
			txt += "\\t"; break;
		case 10:
			txt += "\\n\"\n" + Tabs(tabs) + "\"";
			break;
		case 13:
			if (nor) txt += "\\r"; break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 25:
		case 28:
		case 29:
		case 30:
		case 31:
			CError("Character found not common in strings. Source is binary?");
		case 26:
			CError("byte 26[1a] found. Windows text file termination. Source is binary?");
		case 27:
			// Supported for ANSI 
			txt += "\\x1b";
			break;
		case '"':
			txt += "\\\"";
			break;
		default:
			txt += (char)b;
			break;
		}
	}
	txt += "\";\n";
	string outfile = ifile + ".c";
	string headfile = ifile + ".h";
	if (cpp) {
		outfile = ifile + ".cpp";
		headfile = ifile + ".hpp";
	}
	string ostring{ "" };
	string hstring{ "" };
	if (onamespace.size()) ostring = "namespace " + onamespace + " {\n\t"; hstring = ostring;
	if (!head) ostring += "static "; 
	hstring += "extern ";
	ostring += defi + " = \n" + Tabs(tabs);
	hstring += defi+";\n";
	ostring += txt;
	if (onamespace.size()) { ostring += "}\n"; hstring += "}\n"; }
	if (head) {
		QCol->Doing("Saving", headfile); SaveString(headfile, hstring);
		QCol->Doing("Saving", outfile);	SaveString(outfile, ostring);
	} else {
		QCol->Doing("Saving", headfile);
		SaveString(headfile, ostring);
	}
}

int main(int c, char** a) {
	QCol->Magenta("QETF - Quicky Embed Text File\n");
	QCol->Doing("Coded by", "Jeroen P. Broks");
	QCol->Green("(c) Jeroen P. Broks 2022 - Licensed under the terms of the GPL3\n\n");
	FlagConfig Flags;
	AddFlag_String(Flags, "n", "");
	AddFlag_String(Flags, "v", "StringVar");
	AddFlag_Bool(Flags, "c",false);
	AddFlag_Bool(Flags, "h", false);
	AddFlag_Bool(Flags, "r", false);
	auto Param = ParseArg(c, a, Flags);
	if (!Param.arguments.size()) {
		QCol->Red("Usage: ");
		QCol->Yellow(StripAll(a[0]));
		QCol->Green(" [flags] ");
		QCol->Magenta("<File>\n\n");
		QCol->Yellow("Available flags:\n");
		QCol->Cyan("-n <namespace>   "); QCol->Green("Use NameSpace\n");
		QCol->Cyan("-v <varname>     "); QCol->Green("Name of the string variable\n");
		QCol->Cyan("-c               "); QCol->Green("Use the C++ string class in stead of a character array\n");
		QCol->Cyan("-h               "); QCol->Green("In stead of using one file containing the string as a static var for including, make a source and header seperately\n");
		QCol->Cyan("-r               "); QCol->Green("Keep the <CR> character (will otherwise be filtered out)\n");
		QCol->Reset();
		return 0;
	}
	for (auto p : Param.arguments) Process(p, "", Param.string_flags["n"], Param.string_flags["v"], Param.bool_flags["c"], Param.bool_flags["h"], Param.bool_flags["r"]);

}