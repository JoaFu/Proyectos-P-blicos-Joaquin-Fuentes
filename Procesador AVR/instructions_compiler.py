import traceback
import sys

class AVRCompiler:
    def __init__(self):
        # Mapeo de instrucciones a opcodes base
        self.opcodes = {
            'ADD':  0b0000_0000_0000_0000,
            'AND':  0b0001_0000_0000_0000,
            'SUB':  0b0010_0000_0000_0000,
            'CLR':  0b0011_0000_0000_0000,
            'ANDI': 0b0100_0000_0000_0000,
            'BRBS': 0b0101_0000_0000_0000,
            'CPI':  0b0110_0000_0000_0000,
            'JMP':  0b0111_0000_0000_0000,
            'IN':   0b1000_0000_0000_0000,
            'LDI':  0b1001_0000_0000_0000,
            'LDS':  0b1010_0000_0000_0000,
            'MOV':  0b1011_0000_0000_0000,
            'OUT':  0b1100_0000_0000_0000,
            'STS':  0b1101_0000_0000_0000,
            'CBI':  0b1110_0000_0000_0000,
            'SBI':  0b1111_0000_0000_0000,
        }
        
        # Alias para BRBS según bit del SREG
        self.branch_alias = {
            'BRCS': 0,  # Branch if Carry Set
            'BREQ': 1,  # Branch if Equal (Z=1)
        }
        
        # Definición de puertos y pines
        self.io_ports = {
            # PORTx son outputs, PINx son inputs
            'PORTB': 0x05,   # The PORTB Data Register
            'PINB':  0x03,   # Input B Pins address
            'PORTC': 0x08,   # The PORTC Data Register  
            'PINC':  0x06,   # Input C pins address
            'PORTD': 0x0B,   # The PORTD Data Register
            'PIND':  0x09,   # Input D pins address
            'SREG':  0x3F,   # The AVR Status Register
        }
        
        # Bits del SREG
        self.sreg_bits = {
            'C': 0,  # Carry Bit
            'Z': 1,  # Zero Bit
        }
        
        self.labels = {}
        self.instructions = []
    
    def parse_register(self, reg_str):
        #Convierte 'R16' o 'r16' a número de registro
        reg_str = reg_str.upper().strip()
        if reg_str.startswith('R'):
            return int(reg_str[1:])
        raise ValueError(f"Formato de registro inválido: {reg_str}")
    
    def parse_immediate(self, imm_str):
        #Convierte valores inmediatos (decimal, hex, binario) o nombres de puertos
        imm_str = imm_str.strip().upper()
        
        # Verificar si es un nombre de puerto definido
        if imm_str in self.io_ports:
            return self.io_ports[imm_str]
        
        # Verificar si es un bit del SREG para BRBS
        if imm_str in self.sreg_bits:
            return self.sreg_bits[imm_str]
        
        # Parsear como número
        if imm_str.startswith('0X'):
            return int(imm_str, 16)
        elif imm_str.startswith('0B'):
            return int(imm_str, 2)
        else:
            return int(imm_str)
    
    def encode_ADD(self, rd, rr):
        # ADD Rd, Rr -> 0000 00rd dddd rrrr
        instr = self.opcodes['ADD']
        instr |= ((rd & 0x1F) << 4)  # Rd en bits [8:4]
        instr |= ((rr & 0x10) << 5)  # Bit 4 de Rr va a bit 9
        instr |= (rr & 0x0F)          # Bits [3:0] de Rr
        return instr
    
    def encode_AND(self, rd, rr):
        # AND Rd, Rr -> 0001 00rd dddd rrrr
        instr = self.opcodes['AND']
        instr |= ((rd & 0x1F) << 4)
        instr |= ((rr & 0x10) << 5)
        instr |= (rr & 0x0F)
        return instr
    
    def encode_SUB(self, rd, rr):
        # SUB Rd, Rr -> 0010 00rd dddd rrrr
        instr = self.opcodes['SUB']
        instr |= ((rd & 0x1F) << 4)
        instr |= ((rr & 0x10) << 5)
        instr |= (rr & 0x0F)
        return instr
    
    def encode_CLR(self, rd):
        # CLR Rd -> 0011 00rd dddd dddd (XOR Rd,Rd)
        instr = self.opcodes['CLR']
        instr |= ((rd & 0x1F) << 4)
        instr |= ((rd & 0x10) << 5)
        instr |= (rd & 0x0F)
        return instr
    
    def encode_ANDI(self, rd, k):
        # ANDI Rd, K -> 0100 KKKK dddd KKKK (Rd debe ser R16-R31)
        if rd < 16:
            raise ValueError("ANDI solo acepta R16-R31")
        instr = self.opcodes['ANDI']
        rd_offset = rd - 16
        instr |= ((k & 0xF0) << 4)    # Bits altos de K [11:8]
        instr |= (rd_offset << 4)      # Rd en bits [7:4]
        instr |= (k & 0x0F)            # Bits bajos de K [3:0]
        return instr
    
    def encode_BRBS(self, bit, k):
        # BRBS s, k -> 0101 00kk kkkk ksss
        instr = self.opcodes['BRBS']
        instr |= ((k & 0x7F) << 3)     # k en bits [9:3]
        instr |= (bit & 0x07)           # bit SREG en [2:0]
        return instr
    
    def encode_CPI(self, rd, k):
        # CPI Rd, K -> 0110 KKKK dddd KKKK
        if rd < 16:
            raise ValueError("CPI solo acepta R16-R31")
        instr = self.opcodes['CPI']
        rd_offset = rd - 16
        instr |= ((k & 0xF0) << 4)
        instr |= (rd_offset << 4)
        instr |= (k & 0x0F)
        return instr
    
    def encode_JMP(self, k):
        # JMP k -> 0111 0000 0kkk kkkk
        instr = self.opcodes['JMP']
        instr |= (k & 0x7F)
        return instr
    
    def encode_IN(self, rd, a):
        # IN Rd, A -> 1000 0AAA dddd AAAA
        instr = self.opcodes['IN']
        instr |= ((rd & 0x1F) << 4)
        instr |= ((a & 0x30) << 5)     # Bits [5:4] de A van a [10:9]
        instr |= (a & 0x0F)
        return instr
    
    def encode_LDI(self, rd, k):
        # LDI Rd, K -> 1001 KKKK dddd KKKK
        if rd < 16:
            raise ValueError("LDI solo acepta R16-R31")
        instr = self.opcodes['LDI']
        rd_offset = rd - 16
        instr |= ((k & 0xF0) << 4)
        instr |= (rd_offset << 4)
        instr |= (k & 0x0F)
        return instr
    
    def encode_LDS(self, rd, k):
        # LDS Rd, k -> 1010 KKKK dddd KKKK
        if rd < 16:
            raise ValueError("LDS solo acepta R16-R31")
        instr = self.opcodes['LDS']
        rd_offset = rd - 16
        instr |= ((k & 0xF0) << 4)
        instr |= (rd_offset << 4)
        instr |= (k & 0x0F)
        return instr
    
    def encode_MOV(self, rd, rr):
        # MOV Rd, Rr -> 1011 00rd dddd rrrr
        instr = self.opcodes['MOV']
        instr |= ((rd & 0x1F) << 4)
        instr |= ((rr & 0x10) << 5)
        instr |= (rr & 0x0F)
        return instr
    
    def encode_OUT(self, a, rr):
        # OUT A, Rr -> 1100 0AAA rrrr AAAA
        instr = self.opcodes['OUT']
        instr |= ((rr & 0x1F) << 4)
        instr |= ((a & 0x30) << 5)
        instr |= (a & 0x0F)
        return instr
    
    def encode_STS(self, k, rr):
        # STS k, Rr -> 1101 KKKK rrrr KKKK
        if rr < 16:
            raise ValueError("STS solo acepta R16-R31")
        instr = self.opcodes['STS']
        rr_offset = rr - 16
        instr |= ((k & 0xF0) << 4)
        instr |= (rr_offset << 4)
        instr |= (k & 0x0F)
        return instr
    
    def encode_CBI(self, a, b):
        # CBI A, b -> 1110 AAAA Abbb (clear bit)
        instr = self.opcodes['CBI']
        instr |= ((a & 0x1F) << 3)
        instr |= (b & 0x07)
        return instr
    
    def encode_SBI(self, a, b):
        # SBI A, b -> 1111 AAAA Abbb (set bit)
        instr = self.opcodes['SBI']
        instr |= ((a & 0x1F) << 3)
        instr |= (b & 0x07)
        return instr
    
    def first_pass(self, lines):
        """Primera pasada: detectar labels"""
        address = 0
        for line in lines:
            line = line.split(';')[0].strip()  # Eliminar comentarios
            if not line:
                continue
            
            if ':' in line and not line.startswith(' '):
                # Es un label
                label = line.split(':')[0].strip()
                self.labels[label] = address
            else:
                # Es una instrucción
                address += 1
    
    def second_pass(self, lines):
        """Segunda pasada: ensamblar instrucciones"""
        current_address = 0
        
        for line_num, line in enumerate(lines, 1):
            original_line = line
            line = line.split(';')[0].strip()  # Quita comentarios y espacios extras
            
             # Si la línea está vacía, o es una etiqueta ("label:")
            # y no empieza con espacio (es decir, es una instrucción), la saltamos
            if not line or ':' in line and not line.startswith(' '):
                continue
            try:      # Separa: convierte "ADD R16, R17" → ["ADD", "R16", "R17"]
                parts = line.replace(',', ' ').split()
                mnemonic = parts[0].upper()
                
                if mnemonic in ['BRCS', 'BREQ']:
                    # Alias de BRBS
                    bit = self.branch_alias[mnemonic]
                    label = parts[1]
                    offset = self.labels[label] - (current_address + 1)
                    if offset < -64 or offset > 63:
                        raise ValueError(f"Salto fuera de rango: {offset}")
                    instr = self.encode_BRBS(bit, offset & 0x7F)
                
                elif mnemonic == 'BRBS':
                    # BRBS directo con bit del SREG
                    bit = self.parse_immediate(parts[1])
                    label = parts[2]
                    offset = self.labels[label] - (current_address + 1)
                    if offset < -64 or offset > 63:
                        raise ValueError(f"Salto fuera de rango: {offset}")
                    instr = self.encode_BRBS(bit, offset & 0x7F)
                
                elif mnemonic == 'ADD':
                    rd = self.parse_register(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_ADD(rd, rr)
                
                elif mnemonic == 'AND':
                    rd = self.parse_register(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_AND(rd, rr)
                
                elif mnemonic == 'SUB':
                    rd = self.parse_register(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_SUB(rd, rr)
                
                elif mnemonic == 'CLR':
                    rd = self.parse_register(parts[1])
                    instr = self.encode_CLR(rd)
                
                elif mnemonic == 'ANDI':
                    rd = self.parse_register(parts[1])
                    k = self.parse_immediate(parts[2])
                    instr = self.encode_ANDI(rd, k)
                
                elif mnemonic == 'CPI':
                    rd = self.parse_register(parts[1])
                    k = self.parse_immediate(parts[2])
                    instr = self.encode_CPI(rd, k)
                
                elif mnemonic == 'JMP':
                    if parts[1] in self.labels:
                        k = self.labels[parts[1]]
                    else:
                        k = self.parse_immediate(parts[1])
                    instr = self.encode_JMP(k)
                
                elif mnemonic == 'IN':
                    rd = self.parse_register(parts[1])
                    a = self.parse_immediate(parts[2])
                    instr = self.encode_IN(rd, a)
                
                elif mnemonic == 'LDI':
                    rd = self.parse_register(parts[1])
                    k = self.parse_immediate(parts[2])
                    instr = self.encode_LDI(rd, k)
                
                elif mnemonic == 'LDS':
                    rd = self.parse_register(parts[1])
                    k = self.parse_immediate(parts[2])
                    instr = self.encode_LDS(rd, k)
                
                elif mnemonic == 'MOV':
                    rd = self.parse_register(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_MOV(rd, rr)
                
                elif mnemonic == 'OUT':
                    a = self.parse_immediate(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_OUT(a, rr)
                
                elif mnemonic == 'STS':
                    k = self.parse_immediate(parts[1])
                    rr = self.parse_register(parts[2])
                    instr = self.encode_STS(k, rr)
                
                elif mnemonic == 'CBI':
                    a = self.parse_immediate(parts[1])
                    b = self.parse_immediate(parts[2])
                    instr = self.encode_CBI(a, b)
                
                elif mnemonic == 'SBI':
                    a = self.parse_immediate(parts[1])
                    b = self.parse_immediate(parts[2])
                    instr = self.encode_SBI(a, b)
                
                else:
                    raise ValueError(f"Instrucción desconocida: {mnemonic}")
                
                self.instructions.append(instr)
                current_address += 1
                
            except Exception as e:
                print(f"Error en línea {line_num}: {original_line}")
                print(f"  {e}")
                raise
    
    def compile(self, asm_file, hex_file):
        """Compila archivo .asm a .hex"""
        with open(asm_file, 'r') as f:
            lines = f.readlines()
        
        self.first_pass(lines)
        self.second_pass(lines)
        
        with open(hex_file, 'w') as f:
            for instr in self.instructions:
                f.write(f"{instr:04X}\n")
try:
    asm = sys.argv[1] if len(sys.argv) > 1 else 'programa.asm'
    out = sys.argv[2] if len(sys.argv) > 2 else 'instrucciones.hex'
    c = AVRCompiler()
    c.compile(asm, out)
    print('OK: compile() returned without exception')
except Exception as e:
    print('EXCEPTION in compile:')
    traceback.print_exc()
    sys.exit(2)

