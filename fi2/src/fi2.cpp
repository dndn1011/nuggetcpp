#include <iostream>
#include <cctype>
#include <array>
#include <vector>
#include <assert.h>

#include <functional>

#include "identifier.h"
#include "notice.h"

#include "UI.h"
#include "types.h"


using std::printf;
using namespace identifier;

constexpr size_t maxvars = 256;
constexpr size_t maxmemory = 65536;

std::array<uint8_t, maxvars> variables1;
std::array<uint8_t, maxvars> variables2;
std::array<uint8_t, maxvars> varflags;
std::array <std::vector<uint16_t>, maxvars> triggers;

std::array<uint8_t, maxmemory> memory;

enum class Opcode : int {
	ldr = 0xe0,
	sub,
	ine,
	pop,
	inc,
	str,
	run,
	end,
	imm,
	trg,
};

/*
	ldr : ([last]) => [last]
	sub : [last-1] - [last] => [last]
	ine : if [last-1] != 0 then jump forward [last],pop
	pop : pop [last] off stack
	inc : increment [last]
	str : [last-1] => ([last]),pop
	run : trigger change [last]

	; if(r0!=10) { r0 = r0 + 1 }
	@1000
	0                     ; [0] 
	ldr                   ; (load reg) [contents of register 0]
	10                    ; [contents of 0] [10]
	sub                   ; [contents of 0] [bool]
	3                     ; [contents of 0] [bool] [3]
	ine                   ; [contents of 0]
     pop
	 inc                  ; [contents of 0]+1
	 0					  ; [contents of 0]+1 [0]
	 str                  ; (store reg) [contents of 0]+1
	end

	@2000
    0
	0
	str
	0
	run
*/

void SetRule(const std::vector<uint8_t>& v,int16_t start) {
	uint8_t* ptr = memory.data() + start;
	size_t last = v.size();
	size_t i = 0;
	size_t numTriggers = v[i++];
	assert(numTriggers + 1 < last);
	
	*ptr++ = 0;   // first byte reserved for rule flags
	for (; i < numTriggers+1; i++) {
		triggers[v[i]].push_back(start);
	}
	for (; i < last; i++) {
		*ptr++ = v[i];
	}
}

// Overload unary + on enums to convert to underlying integer type
// this makes sense as the C++ standard performs promotion of integer types
// This allows the use of class enums without the code becoming unreadable
template <typename T>
constexpr auto operator+(T e) noexcept
-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
{
	return static_cast<std::underlying_type_t<T>>(e);
}

void EvalRpn(uint16_t start) {
	std::vector<uint8_t> stack;
	size_t top = 0;
	size_t point = 1;   // skip the rule flags
	uint8_t code = 0;
	while ((code = memory[start+point])!=+Opcode::end) {
		switch ((Opcode)code) {
		case Opcode::imm:
		{
			point++;
			int8_t value = memory[start + point];
			stack.push_back(value);
			point++;
		} break;
		case Opcode::str:
		{
			point++;
			uint8_t reg = memory[start + point];
			variables1[reg] = stack.back();
			point++;
		} break;
		case Opcode::ldr:
		{
			point++;
			uint8_t reg = memory[start + point];
			stack.push_back(variables1[reg]);
			point++;
		} break;
		case Opcode::sub:
		{
			point++;
			int8_t value1 = *(stack.end() - 2);
			int8_t value2 = *(stack.end() - 1);
			stack.back() = value1 - value2;
		} break;
		case Opcode::inc:
		{
			point++;
			int8_t value = *(stack.end() - 1);
			stack.back() = value + 1;
		} break;
		case Opcode::ine:
		{
			point++;
			uint8_t jump = memory[start + point];
			int8_t value = *(stack.end() - 1);
			if (value != 0) {
				point++;
			} else {
				point += ((size_t)jump) + 1;
			}
		} break;
		case Opcode::pop:
		{
			point++;
			stack.pop_back();
		} break;
		default:
			assert(("unimplemented", 0));
		}
	}
}

void RunRule(int16_t start) {
	EvalRpn(start);
}

// returns true if there were changes
bool Execute() {
	size_t changes = 0;
	// copy vars1 -> vars2 flagging differences
	for (size_t index = 0; index < maxvars; index++) {
		uint8_t prev = variables2[index];
		uint8_t next = variables1[index];
		variables2[index] = next;
		if (prev != next) {
			changes++;
			varflags[index] ^= 1;   // toggle bit 0 to indicate the value changed
		}
	}

	// now run rules on dirty values
	for (size_t index = 0; index < maxvars; index++) {
		uint8_t flags = varflags[index];
		for (auto& p : triggers[index]) {
			// check the rule to see if it has been updated yet
			if (((memory[p] ^ flags) & 1) != 0) {
				RunRule(p);
				memory[p] ^= 1;
			}
		}
	}
	return changes > 0;
}


void TestFi()
{
	std::vector <uint8_t > rule1 = {
		1,      // number of dependencies
		0,      // the dependencies (triggered by)

		+Opcode::ldr,
		0,

		+Opcode::imm,
		10,

		+Opcode::sub,

		+Opcode::ine,
		4,

		+Opcode::pop,

		+Opcode::inc,

		+Opcode::str,
		0,

		+Opcode::end
	};
	SetRule(rule1, 0x1000);
	
	std::vector <uint8_t > rule2 = {
		0,

		+Opcode::imm,
		3,

		+Opcode::str,
		0,

		+Opcode::end
	};
	SetRule(rule2, 0x2000);

	RunRule(0x2000);

	while (Execute()) {
		std::printf("%d\n", +variables2[0]);
	}

}

int mainout() {
	UI::Init();

	Notice::Set(ID("C1.color"), nugget::Color(1, 0, 0, 1));

	auto ZZZ = ID("C1");

	UI::Button(ID("Button1"), ID(""), UI::Geometry(100, 100, 200, 50), "1");
	UI::Button(ID("Button2"), ID(""), UI::Geometry(100, 200, 200, 50), "2");
	UI::Circle(ID("C1"), ID(""), UI::Geometry(500, 180, 20, 0));


	Notice::Set(ID("TESTVALUE"), "HELLO");
	printf("%s\n", Notice::GetString(ID("TESTVALUE")).c_str());

	Notice::RegisterHandler(ID("TESTVALUE"), [](IDType id) {
		printf("@@@@@@@@@@@@@@@@@@@ changed @@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("%s\n", Notice::GetString(id).c_str());
		return false;
		});

	Notice::Set(ID("TESTVALUE"), "HELLO2");

	int count = 0;
	const int cycle = 30;
	UI::Exec([&count](){
		//printf("%d\n", Notice::GetInt32(IDCombine({ "Button1", "state" })));
		count++;
		if (count % cycle == 0) {
			if (count / cycle & 1) {
				printf("1\n");
				Notice::Set(ID("C1.color"), nugget::Color(1, 0, 0, 1));
			} else {
				printf("2\n");
				Notice::Set(ID("C1.color"), nugget::Color(1, 1, 0, 1));
			}

		}
	});
	return 0;
}

