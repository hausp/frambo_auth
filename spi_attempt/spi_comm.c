#include <linux/types.h>

uint8_t NRSTPD = 22;

uint8_t MAX_LEN = 16;

uint8_t PCD_IDLE       = 0x00;
uint8_t PCD_AUTHENT    = 0x0E;
uint8_t PCD_TRANSCEIVE = 0x0C;
uint8_t PCD_RESETPHASE = 0x0F;

uint8_t PICC_REQIDL    = 0x26;
uint8_t PICC_ANTICOLL  = 0x93;

uint8_t MI_OK       = 0;
uint8_t MI_NOTAGERR = 1;
uint8_t MI_ERR      = 2;

uint8_t CommandReg     = 0x01;
uint8_t CommIEnReg     = 0x02;
uint8_t CommIrqReg     = 0x04;
uint8_t ErrorReg       = 0x06;
uint8_t FIFODataReg    = 0x09;
uint8_t FIFOLevelReg   = 0x0A;
uint8_t ControlReg     = 0x0C;
uint8_t BitFramingReg  = 0x0D;

uint8_t ModeReg        = 0x11;
uint8_t TxControlReg   = 0x14;
uint8_t TxAutoReg      = 0x15;

uint8_t TModeReg          = 0x2A;
uint8_t TPrescalerReg     = 0x2B;
uint8_t TReloadRegH       = 0x2C;
uint8_t TReloadRegL       = 0x2D;

uint8_t serNum[2];

uint8_t Read_MFRC522(uint8_t addr) {
    uint8_t rx[2];
    uint8_t tx[] = {(addr<<1) & 0x7E | 0x80, 0};

    spi_write(spi_device, &tx, sizeof(tx));
    spi_read(spi_device, &rx, sizeof(rx));

    return rx[1];
}

struct {
    uint8_t status;
    uint8_t* backData;
    uint8_t backLen;
} cardinfo_t;

void Write_MFRC522(uint8_t addr, uint8_t val) {
    uint8_t tx[] = {(addr<<1) & 0x7E, val};

    spi_write(spi_device, &tx, sizeof(tx));
}

void MFRC522_Reset() {
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void SetBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);
}

void ClearMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn() {
    uint8_t tmp = Read_MFRC522(TxControlReg);
    if (~(temp & 0x03)) {
        SetBitMask(TxControlReg, 0x03);
    }
}

struct cardinfo_t MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t dataSize) {
    uint8_t* backData;
    uint8_t backLen;
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint8_t i;

    if (command == PCD_AUTHENT) {
        irqEn = 0x12;
        waitIRq = 0x10;
    } else if (command == PCD_TRANSCEIVE) {
        irqEn = 0x77;
        waitIRq = 0x30;
    }

    Write_MFRC522(CommIEnReg, irqEn|0x80);
    ClearBitMask(CommIrqReg, 0x80);
    SetBitMask(FIFOLevelReg, 0x80);
    Write_MFRC522(CommandReg, PCD_IDLE);

    while (i < dataSize) {
        Write_MFRC522(FIFODataReg, sendData[i]);
        i++;
    }

    Write_MFRC522(CommandReg, command);

    if (command == PCD_TRANSCEIVE) {
        SetBitMask(BitFramingReg, 0x80);
    }
    
    i = 2000;
    while (true) {
        n = Read_MFRC522(CommIrqReg);
        i = i - 1;

        if (~((i!=0) and ~(n&0x01) and ~(n&waitIRq))) {
            break;
        }
    }
    
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0) {
        if ((Read_MFRC522(ErrorReg) & 0x1B)==0x00) {
            status = MI_OK;

            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;
            }
        
            if (command == PCD_TRANSCEIVE) {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits != 0) {
                    backLen = (n-1)*8 + lastBits;
                } else {
                    backLen = n*8;
                }
                
                if (n == 0) {
                    n = 1;
                } else if (n > MAX_LEN) {
                    n = MAX_LEN;
                }

                backdata = kmalloc(n*sizeof(uint8_t), GFP_KERNEL);
    
                i = 0;
                while (i < n) {
                    backdata[i] = Read_MFRC522(FIFODataReg);
                    i++;
                }
            }
        } else {
            status = MI_ERR;
        }
    }

    struct cardinfo_t ret;
    ret.status = status;
    ret.backdata = backData;
    ret.backLen = backLen;

    return ret;
}

void MFRC522_Init() {
    MFRC522_Reset();
    Write_MFRC522(TModeReg, 0x8D);
    Write_MFRC522(TPrescalerReg, 0x3E);
    Write_MFRC522(TReloadRegL, 30);
    Write_MFRC522(TReloadRegH, 0);
    
    Write_MFRC522(TxAutoReg, 0x40);
    Write_MFRC522(ModeReg, 0x3D);
    AntennaOn();
}
