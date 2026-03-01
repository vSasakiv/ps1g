#include "ps1g/utils/disassembler.h"
#include "ps1g/MIPSR3000A.h"

#include <format>

namespace ps1g {

	std::string disassemble(uint32_t raw_instruction, uint32_t pc) {

		MIPSR3000A::Instruction instruction(raw_instruction);

		if (instruction.getPrimaryOpcode() != 0) {
			switch (instruction.getPrimaryOpcode()) {

			// J -> pc = (pc & 0xF0000000) + (imm26 << 2)
			case 0x02: {
				uint32_t imm26 = instruction.getImm26();
				return std::format("J     0x{:08X}", (pc & 0xF0000000) | (imm26 << 2));
			}
			// JAL -> pc = (pc & 0xF0000000) + (imm26 << 2) , ra = pc + 4
			case 0x03: {
				uint32_t imm26 = instruction.getImm26();
				return std::format("JAL   0x{:08X}", (pc & 0xF0000000) | (imm26 << 2));
			}

			// BEQ -> Reg[rs] == Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x04: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("BEQ   r{},r{},0x{:08X}", rt, rs, pc + 4 + (imm16signed << 2));
			}

			// BNE -> Reg[rs] != Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x05: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("BNE   r{},r{},0x{:08X}", rt, rs, pc + 4 + (imm16signed << 2));
			}

			// ADDI -> Reg[rt] = Reg[rs] + imm16signed, raise exception if signed overflow
			case 0x08: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("ADDI  r{},r{},0x{:08X}", rt, rs, imm16signed);
			}

			// ADDIU -> Reg[rt] = Reg[rs] + imm16signed
			case 0x09: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("ADDIU r{},r{},0x{:08X}", rt, rs, imm16signed);
			}

			// ANDI -> Reg[rt] = Reg[rs] & imm16
			case 0x0C: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				return std::format("ANDI  r{},r{},0x{:08X}", rt, rs, imm16);
			}
			
			// ORI -> Reg[rt] = Reg[rs] | imm16
			case 0x0D: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				return std::format("ORI   r{},r{},0x{:08X}", rt, rs, imm16);
			}

			// XORI -> Reg[rt] = Reg[rs] ^ imm16
			case 0x0E: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				return std::format("XORI  r{},r{},0x{:08X}", rt, rs, imm16);
			}

			 // LUI -> Reg[rt] = imm16 << 16
			case 0x0F: {
				uint32_t rt = instruction.getRt();
				uint32_t imm16 = instruction.getImm16();

				return std::format("LUI   r{},0x{:04X}", rt, imm16);
			}

			// COP0 instructions
			case 0x10: {
				uint32_t cop_operation = instruction.getRs();
				switch (cop_operation) {
				// MTF0 -> Reg[rt] = Cop0[rd], load delay
				case 0x00: {
					uint32_t cpu_rt = instruction.getRt();
					uint32_t cop0_rd = instruction.getRd();

					return std::format("MTF0  r{},copr{}", cpu_rt, cop0_rd);
				}

				// MTC0 -> Cop0[rd] = Reg[rt]
				case 0x04: {
					uint32_t cpu_rt = instruction.getRt();
					uint32_t cop0_rd = instruction.getRd();

					return std::format("MTC0  r{},copr{}", cpu_rt, cop0_rd);
				}
				}
				return "???";
			}

			// LB -> Reg[rt] = [imm16signed + Reg[rs]]
			case 0x20: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("LB    r{},0x{:04X}(r{})", rt, imm16signed, rs);
			}
			
			// LW -> Reg[rt] = [imm16signed + Reg[rs]]
			case 0x23: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("LW    r{},0x{:04X}(r{})", rt, imm16signed, rs);
			}

			// SB -> [imm16signed + Reg[rs]] = rt
			case 0x28: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("SB    r{},0x{:04X}(r{})", rt, imm16signed, rs);
			}

			// SH -> [imm16signed + Reg[rs]] = rt
			case 0x29: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("SH    r{},0x{:04X}(r{})", rt, imm16signed, rs);
			}

			// SW -> [imm16signed+Reg[rs]] = rt
			case 0x2B: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				return std::format("SW    r{},0x{:04X}(r{})", rt, imm16signed, rs);
			}

			default:
				return "???";
			}
		}
		else {
			switch (instruction.getSecondaryOpcode()) {
			// SLL -> Reg[rd] = Reg[rt] << (imm16 & 0x1F)
			case 0x00: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm5 = instruction.getImm5();

				return std::format("SLL   r{},r{},0x{:05X}", rt, rs, imm5);
			}

			// SRA -> Reg[rd] = Reg[rt] << (imm16 & 0x1F)
			case 0x03: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm5 = instruction.getImm5();

				return std::format("SRA   r{},r{},0x{:05X}", rt, rs, imm5);
			}

			// JR -> pc = Reg[rs]
			case 0x08: {
				uint32_t rs = instruction.getRs();

				return std::format("JR    r{}", rs);
			}

			// ADD -> Reg[rd] = Reg[rt] + Reg[rs], exception if signed overflow
			case 0x20: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("ADD   r{},r{},r{}", rd, rs, rt);
			}

			// ADDU -> Reg[rd] = Reg[rt] + Reg[rs]
			case 0x21: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("ADDU  r{},r{},r{}", rd, rs, rt);
			}

			// AND -> Reg[rd] = Reg[Rt] & Reg[Rs]
			case 0x24: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("AND   r{},r{},r{}", rd, rs, rt);
			}
		
			// OR -> Reg[rd] = Reg[Rt] | Reg[Rs]
			case 0x25: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("OR    r{},r{},r{}", rd, rs, rt);
			}

			// XOR -> Reg[rd] = Reg[Rt] ^ Reg[Rs]
			case 0x26: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("XOR   r{},r{},r{}", rd, rs, rt);
			}

			// NOR -> Reg[rd] = 0xffffffff ^ (Reg[Rs] | Reg[rt]) 
			case 0x27: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("NOR   r{},r{},r{}", rd, rs, rt);
			}

			// SLTU -> Reg[rd] = Reg[rs] < Reg[rt]
			case 0x2B: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				return std::format("SLTU   r{},r{},r{}", rd, rs, rt);
			}

			default:
				return "???";
			}
		}

		return "???";
	}
}