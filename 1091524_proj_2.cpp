#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory.h>
#include <sstream>

using namespace std;

#define N 0;
#define T 1;

struct Entry
{
	int entry[8] = { 0 };
	string his = "NNN";
	int miss = 0;
};

struct Inst
{
	string inst;
	int entryNum = -1;
};

const string State[4] = { "SN", "WN", "WT", "ST" };
map<string, int> tag;

int reg[32];
int entrySize;
int entryNum = 0;
vector<Entry> entries;
vector<Inst> input;

void output(Inst str, char pred, char real)
{
	cout << str.inst << "\n";
	cout << "Using entry " << str.entryNum << "\n";
	cout << "predict: " << pred << ", real execution: " << real << "\n";
	for (int i = 0; i < entrySize; ++i)
	{
		cout << "entry: " << i << "\n";
		cout << "(" << entries[i].his[0] << entries[i].his[1] << entries[i].his[2] << ", ";
		for (int j = 0; j < 8; ++j)
		{
			cout << State[entries[i].entry[j]];
			if (j != 7)cout << ", ";
		}
		cout << ") " << " misprediction: " << entries[i].miss << "\n";
	}
	cout << "\n";
}

char predict(int entryNum)
{
	int state = entries[entryNum].entry[tag[entries[entryNum].his]];

	switch (state)
	{
	case 0:
	case 1:
		return 'N';

	case 2:
	case 3:
		return 'T';

	default:
		return 'E';
	}
}

void decode(Inst str, int& PC)
{
	stringstream ss;
	int curEntry = input[PC].entryNum;

	//抓instruction
	ss.str(str.inst);
	string ins, buffer;
	ss >> ins;

	int i = 0, pos = 0;
	string rs1, rs2, rd;
	ss >> buffer;

	//抓rd
	while (i != buffer.size() && buffer[i] != ',')++i;
	rd = buffer.substr(pos, i - pos);
	pos = ++i;          

	//抓rs1
	while (i != buffer.size() && buffer[i] != ',')++i;
	rs1 = buffer.substr(pos, i - pos);				  
	pos = ++i;

	//抓rs2
	if (pos < buffer.size())
		rs2 = buffer.substr(pos, buffer.size() - pos);

	char pred = predict(curEntry);
	char real = 'N';
	//對應每個instruction做不同的動作
	if (ins == "add")
	{
		reg[tag[rd]] = reg[tag[rs1]] + reg[tag[rs2]];		
	}
	else if (ins == "sub")
	{
		reg[tag[rd]] = reg[tag[rs1]] - reg[tag[rs2]];		
	}
	else if (ins == "mul")
	{
		reg[tag[rd]] = reg[tag[rs1]] * reg[tag[rs2]];		
	}
	else if (ins == "div")
	{
		reg[tag[rd]] = reg[tag[rs1]] / reg[tag[rs2]];
	}
	else if (ins == "rem")
	{
		reg[tag[rd]] = reg[tag[rs1]] % reg[tag[rs2]];
	}
	else if (ins == "and")
	{
		reg[tag[rd]] = reg[tag[rs1]] & reg[tag[rs2]];		
	}
	else if (ins == "or")
	{
		reg[tag[rd]] = reg[tag[rs1]] | reg[tag[rs2]];
	}
	else if (ins == "xor")
	{
		reg[tag[rd]] = reg[tag[rs1]] ^ reg[tag[rs2]];		
	}
	else if (ins == "beq")
	{
		string label = rs2;
		rs2 = rd;
		if (reg[tag[rs1]] == reg[tag[rs2]])
		{
			real = 'T';
			PC = tag[label];
		}
	}
	else if (ins == "bne")
	{
		string label = rs2;
		rs2 = rd;
		if (reg[tag[rs1]] != reg[tag[rs2]])
		{
			real = 'T';
			PC = tag[label];
		}
	}
	else if (ins == "blt")
	{
		string label = rs2;
		rs2 = rd;
		if (reg[tag[rs1]] < reg[tag[rs2]])
		{
			real = 'T';
			PC = tag[label];
		}
	}
	else if (ins == "bge")
	{
		string label = rs2;
		rs2 = rd;
		if (reg[tag[rs1]] >= reg[tag[rs2]])
		{
			real = 'T';
			PC = tag[label];
		}
	}
	else if (ins == "li")
	{		
		int cons;
		ss.str("");
		ss.clear();
		ss << rs1;
		ss >> cons;

		reg[tag[rd]] = cons;		
	}
	else if (ins == "addi")
	{		
		int cons;
		ss.str("");
		ss.clear();
		ss << rs2;
		ss >> cons;

		reg[tag[rd]] = reg[tag[rs1]] + cons;
	}
	else if (ins == "andi")
	{		
		int cons;
		ss.str("");
		ss.clear();
		ss << rs2;
		ss >> cons;

		reg[tag[rd]] = reg[tag[rs1]] & cons;
	}
	else if (ins == "ori")
	{		
		int cons;
		ss.str("");
		ss.clear();
		ss << rs2;
		ss >> cons;

		reg[tag[rd]] = reg[tag[rs1]] | cons;
	}
	else if (ins == "xori")
	{		
		int cons;
		ss.str("");
		ss.clear();
		ss << rs2;
		ss >> cons;

		reg[tag[rd]] = reg[tag[rs1]] ^ cons;
	}
	else//debug用
	{
		cout << "rrrrr\n";
	}

	if (real != pred)++entries[curEntry].miss;//紀錄misprediction
	output(str, pred, real);

	//更動entry的內容
	if (real == 'T')
		entries[curEntry].entry[tag[entries[curEntry].his]] = min(3, ++entries[curEntry].entry[tag[entries[curEntry].his]]);
	else if(real == 'N')
		entries[curEntry].entry[tag[entries[curEntry].his]] = max(0, --entries[curEntry].entry[tag[entries[curEntry].his]]);

	//更動entry的history
	entries[curEntry].his[0] = entries[curEntry].his[1];
	entries[curEntry].his[1] = entries[curEntry].his[2];
	entries[curEntry].his[2] = real;
}

void execute()
{
	int PC = 0;
	for (int i = 0; input[i].inst != ""; ++i)//先把整個城程式跑過一遍，訂好每個指令用的entry和Label指向的PC
	{
		if (input[i].inst[input[i].inst.size() - 1] == ':')
		{
			string str = input[i].inst.substr(0, input[i].inst.size() - 1);
			tag[str] = i;
		}
		else
		{
			input[i].entryNum = entryNum;
			entryNum = (entryNum == entrySize - 1) ? 0 : entryNum + 1;
		}
	}
	while (PC != input.size() - 1 && input[PC].inst != "")
	{
		string str = input[PC].inst.substr(0, input[PC].inst.size() - 1);
		if (tag.find(str) != tag.end())//是Label的話跳過
		{
			++PC;
			continue;
		}
		decode(input[PC], PC);
		++PC;
	}
}


void init()
{
	memset(reg, 0, sizeof(reg));
	entries.resize(entrySize);
	input.resize(1000);

	tag["NNN"] = 0;
	tag["NNT"] = 1;
	tag["NTN"] = 2;
	tag["NTT"] = 3;
	tag["TNN"] = 4;
	tag["TNT"] = 5;
	tag["TTN"] = 6;
	tag["TTT"] = 7;
	tag["R0"] = 0;
	tag["R1"] = 1;
	tag["R2"] = 2;
	tag["R3"] = 3;
	tag["R4"] = 4;
	tag["R5"] = 5;
	tag["R6"] = 6;
	tag["R7"] = 7;
	tag["R8"] = 8;
	tag["R9"] = 9;
	tag["R10"] = 10;
	tag["R11"] = 11;
	tag["R12"] = 12;
	tag["R13"] = 13;
	tag["R14"] = 14;
	tag["R15"] = 15;
	tag["R16"] = 16;
	tag["R17"] = 17;
	tag["R18"] = 18;
	tag["R19"] = 19;
	tag["R20"] = 20;
	tag["R21"] = 21;
	tag["R22"] = 22;
	tag["R23"] = 23;
	tag["R24"] = 24;
	tag["R25"] = 25;
	tag["R26"] = 26;
	tag["R27"] = 27;
	tag["R28"] = 28;
	tag["R29"] = 29;
	tag["R30"] = 30;
	tag["R31"] = 31;
}

int main()
{
	cout << "Please input entry(entry > 0):\n";
	cin >> entrySize;
	init();
	cin.ignore();
	int num = 0;
	while (getline(cin, input[num].inst) && input[num].inst != "")++num;
	execute();
}