#include "ps1g/MIPSR3000A.h"
#include "ps1g/Bus.h"
#include "ps1g/utils/arith.h"
#include "ps1g/ExceptionType.h"

#include <format>

namespace ps1g {

	MIPSR3000A::MIPSR3000A() {
		this->pc_ = 0xbfc00000;
		this->next_pc_ = this->pc_ + 4;
		this->current_pc_ = 0xbfc00000;
		this->registers_.fill(0xFAAFFAAF); // Fill with random value for debugging
		this->registers_[0] = 0; // Register 0 is always 0
		this->cop0.reset();
		this->hi_ = 0xFAAFFAAF;
		this->lo_ = 0xFAAFFAAF;
		this->branch_ = false;
		this->delay_slot_ = false;
		this->prev_branch_ = false;
	}

	MIPSR3000A::~MIPSR3000A() {}

	void MIPSR3000A::reset() {
		this->pc_ = 0xbfc00000;
		this->next_pc_ = 0xbfc00004;
		this->current_pc_ = 0xbfc00000;
		this->registers_.fill(0xFAAFFAAF); // Fill with random value for debugging
		this->registers_[0] = 0; // Register 0 is always 0
		this->cop0.reset();
		this->load_delay_queue_.clear();
		this->hi_ = 0xFAAFFAAF;
		this->lo_ = 0xFAAFFAAF;
		this->branch_ = false;
		this->delay_slot_ = false;
		this->prev_branch_ = false;
	}

	void MIPSR3000A::step(Bus& bus) {

		this->current_pc_ = this->pc_;
		this->prev_branch_ = this->delay_slot_;

		if (this->pc_ % 4 != 0) {
			this->exception(ExceptionType::LoadAddressError);
			return;
		}

		Instruction instruction(bus.readU32(this->pc_));

		this->delay_slot_ = this->branch_;
		this->branch_ = false;

		// default + 4
		this->pc_ = this->next_pc_;
		this->next_pc_ += 4;

		if (instruction.getPrimaryOpcode() != 0) {
			switch (instruction.getPrimaryOpcode()) {

			// BcondZ instructions
			case 0x01: {
				uint32_t branch_type = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				switch (branch_type) {
				// BLTZ Reg[rs] < 0 then pc = pc + 4 + (imm16signed << 2)
				case 0x00: {
					if (static_cast<int32_t>(this->readReg(rs)) < 0)
						this->branch(this->pc_ + (imm16signed << 2));
					break;
				}
				// BGEZ Reg[rs] >= 0 then pc = pc + 4 + (imm16signed << 2)
				case 0x01: {
					if (static_cast<int32_t>(this->readReg(rs)) >= 0)
						this->branch(this->pc_ + (imm16signed << 2));
					break;
				}
				// BLTZAL ra = pc + 4 ; Reg[rs] < 0 then pc = pc + 4 + (imm16signed << 2)
				case 0x10: {
					this->writeReg(31, this->pc_ + 4);
					if (static_cast<int32_t>(this->readReg(rs)) < 0)
						this->branch(this->pc_ + (imm16signed << 2));
					break;
				}
				// BGEZAL ra = pc + 4 ; Reg[rs] >= 0 then pc = pc + 4 + (imm16signed << 2)
				case 0x11: {
					this->writeReg(31, this->pc_ + 4);
					if (static_cast<int32_t>(this->readReg(rs)) >= 0)
						this->branch(this->pc_ + (imm16signed << 2));
					break;
				}
				default: {
					std::cout << "Invalid Branch Type" << std::endl;
					throw std::runtime_error("Invalid Branch Type");
				}
				}
				break;
			}

			// J -> pc = (pc & 0xF0000000) + (imm26 << 2)
			case 0x02: {
				uint32_t imm26 = instruction.getImm26();
				this->branch((this->pc_ & 0xF0000000) | (imm26 << 2));
				break;
			}

			// JAL -> pc = (pc & 0xF0000000) + (imm26 << 2) , ra = pc + 4
			case 0x03: {
				uint32_t imm26 = instruction.getImm26();

				this->writeReg(31, this->pc_ + 4);
				this->branch((this->pc_ & 0xF0000000) | (imm26 << 2));
				break;
			}

			// BEQ -> Reg[rs] == Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x04: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (this->readReg(rt) == this->readReg(rs))
					this->branch(this->pc_ + (imm16signed << 2));
				break;
			}

			// BNE -> Reg[rs] != Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x05: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (this->readReg(rt) != this->readReg(rs))
					this->branch(this->pc_ + (imm16signed << 2));
				break;
			}

			// BLEZ -> Reg[rs] <= 0 then pc = pc + 4 + (imm16signed << 2)
			case 0x06: {
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (static_cast<int32_t>(this->readReg(rs)) <= 0)
					this->branch(this->pc_ + (imm16signed << 2));
				break;
			}

			// BGTZ -> Reg[rs] > 0 then pc = pc + 4 + (imm16signed << 2)
			case 0x07: {
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (static_cast<int32_t>(this->readReg(rs)) > 0)
					this->branch(this->pc_ + (imm16signed << 2));
				break;
			}

			// ADDI -> Reg[rt] = Reg[rs] + imm16signed, raise exception if signed overflow
			case 0x08: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (add_signed_overflow(static_cast<int32_t>(this->readReg(rs)), static_cast<int32_t>(imm16signed)))
					this->exception(ExceptionType::ArithmeticOverflow);
				else
					this->writeReg(rt, this->readReg(rs) + imm16signed);
				break;

			}

			// ADDIU -> Reg[rt] = Reg[rs] + imm16signed
			case 0x09: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();
				
				this->writeReg(rt, this->readReg(rs) + imm16signed);
				break;
			}

			// SLTI -> Reg[rt] = Reg[rs] < imm16signed
			case 0x0A: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				this->writeReg(rt, static_cast<int32_t>(this->readReg(rs)) <  static_cast<int32_t>(imm16signed));
				break;
			}

			// SLTIU -> Reg[rt] = Reg[rs] < imm16
			case 0x0B: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16Signed();

				this->writeReg(rt, this->readReg(rs) < imm16);
				break;
			}

			// ANDI -> Reg[rt] = Reg[rs] & imm16
			case 0x0C: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) & imm16);
				break;
			}
			
			// ORI -> Reg[rt] = Reg[rs] | imm16
			case 0x0D: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) | imm16);
				break;
			}

			// XORI -> Reg[rt] = Reg[rs] ^ imm16
			case 0x0E: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) ^ imm16);
				break;
			}

			 // LUI -> Reg[rt] = imm16 << 16
			case 0x0F: {
				uint32_t rt = instruction.getRt();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, imm16 << 16);
				break;
			}

			// COP0 instructions
			case 0x10: {
				cop0Execute(instruction);
				break;
			}

			// LB -> Reg[rt] = [imm16signed + Reg[rs]]
			case 0x20: {
				// If cache is isolated, just void the read for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Read with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					this->load_delay_queue_.push_back(LoadDelay(rt, static_cast<int8_t>(bus.readU8(addr)), 1));
				}
				break;
			}
			
			// LW -> Reg[rt] = [imm16signed + Reg[rs]]
			case 0x23: {
				// If cache is isolated, just void the read for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Read with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					if (addr % 4 == 0)
						this->load_delay_queue_.push_back(LoadDelay(rt, bus.readU32(addr), 1));
					else
						this->exception(ExceptionType::LoadAddressError);
				}
				break;
			}

			// LBU -> Reg[rt] = [imm16signed + Reg[rs]]
			case 0x24: {
				// If cache is isolated, just void the read for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Read with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					this->load_delay_queue_.push_back(LoadDelay(rt, bus.readU8(addr), 1));
				}
				break;
			}

			// SB -> [imm16signed + Reg[rs]] = rt
			case 0x28: {
				// If cache is isolated, just void the write for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Write with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					bus.writeU8(imm16signed + this->readReg(rs), static_cast<uint8_t>(this->readReg(rt)));
				}
				break;
			}

			// SH -> [imm16signed + Reg[rs]] = rt
			case 0x29: {
				// If cache is isolated, just void the write for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Write with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					if (addr % 2 == 0)
						bus.writeU16(imm16signed + this->readReg(rs), static_cast<uint16_t>(this->readReg(rt)));
					else
						this->exception(ExceptionType::StoreAddressError);
				}
				break;
			}

			// SW -> [imm16signed+Reg[rs]] = rt
			case 0x2B: {
				// If cache is isolated, just void the write for now since cache is not implemented
				if (this->cop0.system_status & 0x10000) {
					std::cout << "Write with isolated cache, ignoring" << std::endl;
				}
				else {
					uint32_t rt = instruction.getRt();
					uint32_t rs = instruction.getRs();
					uint32_t imm16signed = instruction.getImm16Signed();

					uint32_t addr = imm16signed + this->readReg(rs);
					if (addr % 4 == 0)
						bus.writeU32(imm16signed + this->readReg(rs), this->readReg(rt));
					else
						this->exception(ExceptionType::StoreAddressError);
				}
				break;
			}

			default:
				std::cout << "Primary Opcode not implemented/non existent: " 
					<< std::hex
					<< std::setfill('0')
					<< std::setw(2)
					<< instruction.getPrimaryOpcode() << std::dec << std::endl;
				throw std::runtime_error("Invalid Primary Opcode");
				break;
			}
		}
		else {
			switch (instruction.getSecondaryOpcode()) {
			// SLL -> Reg[rd] = Reg[rt] << imm5
			case 0x00: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t imm5 = instruction.getImm5();

				this->writeReg(rd, this->readReg(rt) << imm5);
				break;
			}

			// SRL -> Reg[rd] = Reg[rt] >> imm5
			case 0x02: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t imm5 = instruction.getImm5();

				this->writeReg(rd, this->readReg(rt) >> imm5);
				break;
			}

			// SRA -> Reg[rd] = Reg[rt] >>> imm5
			case 0x03: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t imm5 = instruction.getImm5();

				this->writeReg(rd, static_cast<int32_t>(this->readReg(rt)) >> imm5);
				break;
			}

			// JR -> pc = Reg[rs]
			case 0x08: {
				uint32_t rs = instruction.getRs();
				this->branch(this->readReg(rs));
				break;
			}

			// JALR -> Reg[rd] = pc + 4, pc = Reg[rs]
			case 0x09: {
				uint32_t rs = instruction.getRs();
				uint32_t rd = instruction.getRd();

				this->writeReg(rd, this->pc_ + 4);
				this->branch(this->readReg(rs));
				break;
			}

			// Syscall -> Generate Exception
			case 0x0C: {
				this->exception(ExceptionType::Syscall);
				break;
			}

			// MFHI -> Reg[rd] = hi
			case 0x10: {
				uint32_t rd = instruction.getRd();
				this->writeReg(rd, this->readHi());
				break;
			}

			// MTHI -> hi = Reg[rd] 
			case 0x11: {
				uint32_t rs = instruction.getRs();
				this->writeHi(this->readReg(rs));
				break;
			}

			// MFLO -> Reg[rd] = lo
			case 0x12: {
				uint32_t rd = instruction.getRd();
				this->writeReg(rd, this->readLo());
				break;
			}

			// MTLO -> lo = Reg[rd] 
			case 0x13: {
				uint32_t rs = instruction.getRs();
				this->writeLo(this->readReg(rs));
				break;
			}

			// DIV -> lo = Reg[rs] / Reg[rt] ; hi = Reg[rs] % Reg[rt]
			case 0x1A: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();

				int32_t numerator = this->readReg(rs);
				int32_t denominator = this->readReg(rt);

				// Division by 0
				if (denominator == 0) {
					this->writeLo(numerator >= 0 ? -1 : 1);
					this->writeHi(numerator);
				}
				// Result is not possible to represent in 32 bit signed
				else if (numerator == 0x80000000 && denominator == -1) {
					this->writeLo(numerator);
					this->writeHi(0);
				}
				// Normal division
				else {
					this->writeLo(numerator / denominator);
					this->writeHi(numerator % denominator);
				}
				break;
			}

			// DIVU -> lo = Reg[rs] / Reg[rt] ; hi = Reg[rs] % Reg[rt]
			case 0x1B: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();

				uint32_t numerator = this->readReg(rs);
				uint32_t denominator = this->readReg(rt);

				// Division by 0
				if (denominator == 0) {
					this->writeLo(0xFFFFFFFF);
					this->writeHi(numerator);
				}
				// Normal division
				else {
					this->writeLo(numerator / denominator);
					this->writeHi(numerator % denominator);
				}
				break;
			}


			// ADD -> Reg[rd] = Reg[rt] + Reg[rs], exception if signed overflow
			case 0x20: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				if (add_signed_overflow(static_cast<int32_t>(this->readReg(rs)), static_cast<int32_t>(this->readReg(rt))))
					this->exception(ExceptionType::ArithmeticOverflow);
				else
					this->writeReg(rd, this->readReg(rt) + this->readReg(rs));

				break;
			}

			// ADDU -> Reg[rd] = Reg[rt] + Reg[rs]
			case 0x21: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) + this->readReg(rs));
				break;
			}

			// SUBU -> Reg[rd] = Reg[rs] - Reg[rt]
			case 0x23: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rs) - this->readReg(rt));
				break;
			}

			// AND -> Reg[rd] = Reg[Rt] & Reg[Rs]
			case 0x24: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) & this->readReg(rs));
				break;
			}
		
			// OR -> Reg[rd] = Reg[Rt] | Reg[Rs]
			case 0x25: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) | this->readReg(rs));
				break;
			}

			// XOR -> Reg[rd] = Reg[Rt] ^ Reg[Rs]
			case 0x26: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) ^ this->readReg(rs));
				break;
			}

			// NOR -> Reg[rd] = 0xffffffff ^ (Reg[Rs] | Reg[rt]) 
			case 0x27: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, 0xFFFFFFFF ^ (this->readReg(rt) | this->readReg(rs)));
				break;
			}
					 
			// SLT -> Reg[rd] = Reg[rs] < Reg[rt]
			case 0x2A: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, static_cast<int32_t>(this->readReg(rs)) < static_cast<int32_t>(this->readReg(rt)));
				break;
			}

			// SLTU -> Reg[rd] = Reg[rs] < Reg[rt]
			case 0x2B: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rs) < this->readReg(rt));
				break;
			}

			default:
				std::cout << "Secondary Opcode not implemented/non existent: " 
					<< std::hex
					<< std::setfill('0')
					<< std::setw(2)
					<< instruction.getSecondaryOpcode() << std::dec << std::endl;
				throw std::runtime_error("Invalid Secondary Opcode");
				break;
			}
		}
		this->evaluateLoadDelays();
	}

	void MIPSR3000A::cop0Execute(Instruction& instruction)
	{
		uint32_t cop_operation = instruction.getRs();
		switch (cop_operation) {
		// MFC0 -> Reg[rt] = Cop0[rd], load delay
		case 0x00: {
			uint32_t cpu_rt = instruction.getRt();
			uint32_t cop0_rd = instruction.getRd();
			this->load_delay_queue_.push_back(LoadDelay(cpu_rt, this->readCP0(cop0_rd), 1));
			break;
		}
		// MTC0 -> Cop0[rd] = Reg[rt]
		case 0x04: {
			uint32_t cpu_rt = instruction.getRt();
			uint32_t cop0_rd = instruction.getRd();
			this->writeCP0(cop0_rd, this->readReg(cpu_rt));
			break;
		}

		// RFE -> Return from exception, restore previous mode from SR
		case 0x10: {
			if (instruction.getSecondaryOpcode() != 0x10) {
				std::cout << "COP0 operation illegal" << std::hex << cop_operation << std::dec << std::endl;
				throw std::runtime_error("Invalid COP0 operation");
			}

			// Pop processor mode
			uint32_t sr = this->readCP0(MIPSR3000A::CP0::SR);
			uint32_t mode = sr & 0x3F;
			sr = sr & (~0x3F); // zeroes 6 last bits
			sr |= (mode >> 2) & 0x3F; // sets last 6 bits
			this->writeCP0(MIPSR3000A::CP0::SR, sr);
			break;
		}

		default:
			std::cout << "COP0 operation not implemented/existent " << std::hex << cop_operation << std::dec << std::endl;
			throw std::runtime_error("Invalid COP0 operation");
			break;
		}
	}

	void MIPSR3000A::exception(ExceptionType type) {
		uint32_t vector_addr;

		uint32_t sr = this->readCP0(MIPSR3000A::CP0::SR);
		// BEV bit to define where to jump
		if (((sr >> 22)&0x1) == 1) {
			vector_addr = 0xbfc00180; // Rom vector
		}
		else {
			vector_addr = 0x80000080; // Ram vector
		}

		// Shift lower 6 bits of SR to the left - update mode and its previous values
		uint32_t mode = sr & 0x3F;
		sr = sr & (~0x3F); // zeroes 6 last bits
		sr |= (mode << 2) & 0x3F; // sets last 6 bits
		this->writeCP0(MIPSR3000A::CP0::SR, sr);

		// update cause register
		this->writeCP0(MIPSR3000A::CP0::Cause, static_cast<uint32_t>(type) << 2);

		// Exception return PC
		if (this->delay_slot_) {
			this->writeCP0(MIPSR3000A::CP0::EPC, this->current_pc_ - 4);
			this->writeCP0(MIPSR3000A::CP0::Cause, this->readCP0(MIPSR3000A::CP0::Cause) | 0x80000000); // Set BD bit
		}
		else {
			this->writeCP0(MIPSR3000A::CP0::EPC, this->current_pc_);
		}

		this->pc_ = vector_addr;
		this->next_pc_ = vector_addr + 4;
	}

	void MIPSR3000A::writeCP0(uint32_t reg, uint32_t value) {
		switch (reg) {
		case 3:
		case 5:
		case 6:
		case 7:
		case 9:
		case 11: {
			std::cout << "Writes to Debug registers not implemented" << reg << std::dec << std::endl;
			break;
		}
		case 12:
			this->cop0.system_status = value;
			break;
		case 13:
			this->cop0.cause = value;
			break;
		case 14:
			this->cop0.exception_pc = value;
			break;
		default:
			std::cout << "COP0 register not implemented/existent " << reg << std::dec << std::endl;
			throw std::runtime_error("Invalid COP0 register");
			break;
		}
	}

	uint32_t MIPSR3000A::readCP0(uint32_t reg) const {
		switch (reg) {
		case 3:
		case 5:
		case 6:
		case 7:
		case 9:
		case 11: {
			std::cout << "Reads from Debug registers not implemented" << reg << std::dec << std::endl;
			break;
		}
		case 12:
			return this->cop0.system_status;
			break;
		case 13:
			return this->cop0.cause;
			break;
		case 14:
			return this->cop0.exception_pc;
			break;
		default:
			std::cout << "COP0 register not implemented/existent " << reg << std::dec << std::endl;
			throw std::runtime_error("Invalid COP0 register");
			break;
		}
	}

	void MIPSR3000A::branch(uint32_t address) {
		this->next_pc_ = address;
		this->branch_ = true;
	}

	void MIPSR3000A::evaluateLoadDelays() {
		// Execute all delayed loads
		for (LoadDelay &ld : this->load_delay_queue_) {
			if (ld.remaining_duration == 0) {
				this->writeReg(ld.reg, ld.value);
			}
		}

		// Erase all of the delayed loads
		std::erase_if(this->load_delay_queue_, [](const LoadDelay& ld) {
			return ld.remaining_duration == 0;
		});

		// Reduces duration of remaining queue items
		for (LoadDelay &ld : this->load_delay_queue_) {
			ld.remaining_duration--;
		}
	}

	void MIPSR3000A::writeReg(size_t reg, uint32_t value) {
		if (reg == 0)
			std::cout << "Attempted write at register r0, ignoring" << std::endl;

		// If an operation is about to write in a register that is due for a load, the operation takes priority, load is killed
		std::erase_if(this->load_delay_queue_, [reg](const LoadDelay& ld) {
			return ld.remaining_duration == 0 && ld.reg == reg;
		});

		this->registers_[reg] = value;
	}

	uint32_t MIPSR3000A::readReg(size_t reg) {
		return this->registers_[reg];
	}

	void MIPSR3000A::writeLo(uint32_t value) {
		this->lo_ = value;
	}
	uint32_t MIPSR3000A::readLo() {
		return this->lo_;
	}
	void MIPSR3000A::writeHi(uint32_t value) {
		this->hi_ = value;
	}
	uint32_t MIPSR3000A::readHi() {
		return this->hi_;
	}
	
	void MIPSR3000A::rollBack() {
		this->next_pc_ = this->pc_;
		this->pc_ = this->current_pc_;

		this->branch_ = this->delay_slot_;
		this->delay_slot_ = this->prev_branch_;
	}

}
