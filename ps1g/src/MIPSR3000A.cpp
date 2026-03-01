#include "ps1g/MIPSR3000A.h"
#include "ps1g/Bus.h"
#include "ps1g/utils/arith.h"

namespace ps1g {

	MIPSR3000A::MIPSR3000A() {
		this->pc_ = 0xbfc00000;
		this->prev_pc_ = 0x0;
		this->registers_.fill(0xFAAFFAAF); // Fill with random value for debugging
		this->registers_[0] = 0; // Register 0 is always 0
		this->fetched_next_ = 0x0; // NOP
		this->cop0.reset();
		this->hi_ = 0xFAAFFAAF;
		this->lo_ = 0xFAAFFAAF;
	}

	MIPSR3000A::~MIPSR3000A() {}

	void MIPSR3000A::reset() {
		this->pc_ = 0xbfc00000;
		this->prev_pc_ = 0x0;
		this->registers_.fill(0xFAAFFAAF); // Fill with random value for debugging
		this->registers_[0] = 0; // Register 0 is always 0
		this->fetched_next_ = 0x0; // NOP
		this->cop0.reset();
		this->load_delay_queue_.clear();
		this->hi_ = 0xFAAFFAAF;
		this->lo_ = 0xFAAFFAAF;
	}

	void MIPSR3000A::step(Bus& bus) {

		Instruction instruction(this->fetched_next_);
		this->prev_pc_ = pc_;

		std::cout << "Executing instruction: "
			<< std::hex
			<< std::hex
			<< std::setfill('0')
			<< std::setw(8)
			<< this->fetched_next_ << std::dec << std::endl;

		uint32_t instruction_raw = bus.readU32(this->pc_);
		this->fetched_next_ = instruction_raw;

		std::cout << "Fetched next instruction: "
			<< std::hex
			<< std::setfill('0')
			<< std::setw(8)
			<< instruction_raw << " at PC: " << this->pc_ << std::dec << std::endl;


		if (instruction.getPrimaryOpcode() != 0) {
			switch (instruction.getPrimaryOpcode()) {

			// J -> pc = (pc & 0xF0000000) + (imm26 << 2)
			case 0x02: {
				uint32_t imm26 = instruction.getImm26();

				this->pc_ = (this->pc_ & 0xF0000000) | (imm26 << 2);
				break;
			}

			// JAL -> pc = (pc & 0xF0000000) + (imm26 << 2) , ra = pc + 4
			case 0x03: {
				uint32_t imm26 = instruction.getImm26();

				this->writeReg(31, this->pc_ + 4);
				this->pc_ = (this->pc_ & 0xF0000000) | (imm26 << 2);
				break;
			}

			// BEQ -> Reg[rs] == Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x04: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (this->readReg(rt) == this->readReg(rs))
					this->pc_ = this->pc_ + (imm16signed << 2);
				else
					this->pc_ += 4;
				break;
			}

			// BNE -> Reg[rs] != Reg[rt] then pc = pc + 4 + (imm16signed << 2)
			case 0x05: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (this->readReg(rt) != this->readReg(rs))
					this->pc_ = this->pc_ + (imm16signed << 2);
				else
					this->pc_ += 4;
				break;
			}

			// ADDI -> Reg[rt] = Reg[rs] + imm16signed, raise exception if signed overflow
			case 0x08: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();

				if (add_signed_overflow((int32_t)this->readReg(rs), (int32_t)imm16signed))
					throw std::runtime_error("Signed overflow exception not yet implemented");
				else
					this->writeReg(rt, this->readReg(rs) + imm16signed);

				this->pc_ += 0x4;
				break;

			}

			// ADDIU -> Reg[rt] = Reg[rs] + imm16signed
			case 0x09: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16signed = instruction.getImm16Signed();
				
				this->writeReg(rt, this->readReg(rs) + imm16signed);
				this->pc_ += 0x4;
				break;
			}

			// ANDI -> Reg[rt] = Reg[rs] & imm16
			case 0x0C: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) & imm16);
				this->pc_ += 0x4;
				break;
			}
			
			// ORI -> Reg[rt] = Reg[rs] | imm16
			case 0x0D: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) | imm16);
				this->pc_ += 0x4;
				break;
			}

			// XORI -> Reg[rt] = Reg[rs] ^ imm16
			case 0x0E: {
				uint32_t rt = instruction.getRt();
				uint32_t rs = instruction.getRs();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, this->readReg(rs) ^ imm16);
				this->pc_ += 0x4;
				break;
			}

			 // LUI -> Reg[rt] = imm16 << 16
			case 0x0F: {
				uint32_t rt = instruction.getRt();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rt, imm16 << 16);
				this->pc_ += 0x4;
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
					this->load_delay_queue_.push_back(LoadDelay(rt, (int8_t)bus.readU8(addr), 1));
				}
				this->pc_ += 0x4;
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
					this->load_delay_queue_.push_back(LoadDelay(rt, bus.readU32(addr), 1));
				}
				this->pc_ += 0x4;
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
					bus.writeU8(imm16signed + this->readReg(rs), (uint8_t)this->readReg(rt));
				}
				this->pc_ += 0x4;
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
					bus.writeU16(imm16signed + this->readReg(rs), (uint16_t)this->readReg(rt));
				}
				this->pc_ += 0x4;
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
					bus.writeU32(imm16signed + this->readReg(rs), this->readReg(rt));
				}
				this->pc_ += 0x4;
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
			// SLL -> Reg[rd] = Reg[rt] << (imm16 & 0x1F)
			case 0x00: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t imm16 = instruction.getImm16();

				this->writeReg(rd, this->readReg(rt) << (imm16 & 0x1F));
				this->pc_ += 0x4;
				break;
			}

			// JR -> pc = Reg[rs]
			case 0x08: {
				uint32_t rs = instruction.getRs();
				this->pc_ = this->readReg(rs);
				break;
			}

			// ADD -> Reg[rd] = Reg[rt] + Reg[rs], exception if signed overflow
			case 0x20: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				if (add_signed_overflow((int32_t)this->readReg(rs), (int32_t)this->readReg(rt)))
					throw std::runtime_error("Signed overflow exception not yet implemented");
				else
					this->writeReg(rd, this->readReg(rt) + this->readReg(rs));

				this->pc_ += 0x4;
				break;
			}

			// ADDU -> Reg[rd] = Reg[rt] + Reg[rs]
			case 0x21: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) + this->readReg(rs));
				this->pc_ += 0x4;
				break;
			}

			// AND -> Reg[rd] = Reg[Rt] & Reg[Rs]
			case 0x24: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) & this->readReg(rs));
				this->pc_ += 0x4;
				break;
			}
		
			// OR -> Reg[rd] = Reg[Rt] | Reg[Rs]
			case 0x25: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) | this->readReg(rs));
				this->pc_ += 0x4;
				break;
			}

			// XOR -> Reg[rd] = Reg[Rt] ^ Reg[Rs]
			case 0x26: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rt) ^ this->readReg(rs));
				this->pc_ += 0x4;
				break;
			}

			// NOR -> Reg[rd] = 0xffffffff ^ (Reg[Rs] | Reg[rt]) 
			case 0x27: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, 0xFFFFFFFF ^ (this->readReg(rt) | this->readReg(rs)));
				this->pc_ += 0x4;
				break;
			}

			// SLTU -> Reg[rd] = Reg[rs] < Reg[rt]
			case 0x2B: {
				uint32_t rt = instruction.getRt();
				uint32_t rd = instruction.getRd();
				uint32_t rs = instruction.getRs();

				this->writeReg(rd, this->readReg(rs) < this->readReg(rt));
				this->pc_ += 0x4;
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

		std::cout << "===========================" << std::endl;
	}

	void MIPSR3000A::cop0Execute(Instruction& instruction)
	{
		uint32_t cop_operation = instruction.getRs();
		switch (cop_operation) {
		// MTF0 -> Reg[rt] = Cop0[rd], load delay
		case 0x00: {
			uint32_t cpu_rt = instruction.getRt();
			uint32_t cop0_rd = instruction.getRd();

			switch (cop0_rd) {
			case 3:
			case 5:
			case 6:
			case 7:
			case 9:
			case 11: {
				std::cout << "Reads from Debug registers not implemented" << cop0_rd << std::dec << std::endl;
				this->pc_ += 0x04;
				break;
			}

			case 12:
				this->load_delay_queue_.push_back(LoadDelay(cpu_rt, cop0.system_status, 1));
				this->pc_ += 0x4;
				break;

			case 13:
				std::cout << "Reads to Cause register not implemented" << cop0_rd << std::dec << std::endl;
				this->pc_ += 0x04;
				break;

			default:
				std::cout << "COP0 register not implemented/existent " << cop0_rd << std::dec << std::endl;
				throw std::runtime_error("Invalid COP0 register");
				break;
			}
			break;
		}
		// MTC0 -> Cop0[rd] = Reg[rt]
		case 0x04: {
			uint32_t cpu_rt = instruction.getRt();
			uint32_t cop0_rd = instruction.getRd();

			switch (cop0_rd) {
			case 3:
			case 5:
			case 6:
			case 7:
			case 9:
			case 11: {
				std::cout << "Writes to Debug registers not implemented" << cop0_rd << std::dec << std::endl;
				this->pc_ += 0x04;
				break;
			}

			case 12:
				this->cop0.system_status = this->readReg(cpu_rt);
				this->pc_ += 0x4;
				break;

			case 13:
				std::cout << "Writes to Cause register not implemented" << cop0_rd << std::dec << std::endl;
				this->pc_ += 0x04;
				break;

			default:
				std::cout << "COP0 register not implemented/existent " << cop0_rd << std::dec << std::endl;
				throw std::runtime_error("Invalid COP0 register");
				break;
			}

			break;
		}

		default:
			std::cout << "COP0 operation not implemented/existent " << std::hex << cop_operation << std::dec << std::endl;
			throw std::runtime_error("Invalid COP0 operation");
			break;
		}
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

}
