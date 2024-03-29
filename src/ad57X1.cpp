
#include "ad57X1.h"

// Use CPOL = 0,  CPHA = 1 for ADI DACs
AD57X1::AD57X1(uint8_t cs_pin, SPIClass* _spi, const uint8_t _VALUE_OFFSET, uint32_t spiClockFrequency, uint8_t ldac_pin) : VALUE_OFFSET(_VALUE_OFFSET), spi(_spi), cs_pin(cs_pin), ldac_pin(ldac_pin), spi_settings(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE1)) {
}

void AD57X1::writeSPI(uint32_t value) {
  this->spi->beginTransaction(this->spi_settings);
  digitalWrite (this->cs_pin, LOW);

  this->spi->transfer((value >> 16) & 0xFF);
  this->spi->transfer((value >> 8) & 0xFF);
  this->spi->transfer((value >> 0) & 0xFF);

  digitalWrite(this->cs_pin, HIGH);
  this->spi->endTransaction();
}

void AD57X1::updateControlRegister() {
  this->writeSPI(this->WRITE_REGISTERS | this->CONTROL_REGISTER | this->controlRegister);
}

void AD57X1::reset() {
  this->enableOutput();
}

// value is an 18 or 20 bit value
void AD57X1::setValue(uint32_t value) {
  uint32_t command = this->WRITE_REGISTERS | this->DAC_REGISTER | ((value << this->VALUE_OFFSET) & 0xFFFFF);

  this->writeSPI(command);
  this->writeSPI(this->SW_CONTROL_REGISTER | this->SW_CONTROL_LDAC);
}

void AD57X1::enableOutput() {
  this->setOutputClamp(false);
  this->setTristateMode(false);
  this->updateControlRegister();
}

void AD57X1::setInternalAmplifier(bool enable) {
  // (1 << this->RBUF_REGISTER) : internal amplifier is disabled (default)
  // (0 << this->RBUF_REGISTER) : internal amplifier is enabled
  this->controlRegister = (this->controlRegister & ~(1 << this->RBUF_REGISTER)) | (!enable << this->RBUF_REGISTER);
}

// Setting this to enabled will overrule the tristate mode and clamp the output to GND
void AD57X1::setOutputClamp(bool enable) {
  // (1 << this->OUTPUT_CLAMP_TO_GND_REGISTER) : the output is clamped to GND (default)
  // (0 << this->OUTPUT_CLAMP_TO_GND_REGISTER) : the dac is in normal mode
  this->controlRegister = (this->controlRegister & ~(1 << this->OUTPUT_CLAMP_TO_GND_REGISTER)) | (enable << this->OUTPUT_CLAMP_TO_GND_REGISTER);
}

void AD57X1::setTristateMode(bool enable) {
  // (1 << this->OUTPUT_TRISTATE_REGISTER) : the dac output is in tristate mode (default)
  // (0 << this->OUTPUT_TRISTATE_REGISTER) : the dac is in normal mode
  this->controlRegister = (this->controlRegister & ~(1 << this->OUTPUT_TRISTATE_REGISTER)) | (enable << this->OUTPUT_TRISTATE_REGISTER);
}

void AD57X1::setOffsetBinaryEncoding(bool enable) {
  // (1 << this->OFFSET_BINARY_REGISTER) : the dac uses offset binary encoding, should be used when writing unsigned ints
  // (0 << this->OFFSET_BINARY_REGISTER) : the dac uses 2s complement encoding, should be used when writing signed ints (default)
  this->controlRegister = (this->controlRegister & ~(1 << this->OFFSET_BINARY_REGISTER)) | (enable << this->OFFSET_BINARY_REGISTER);
}

/* Linearity error compensation
 * 
 */
// enable = 0 -> Range 0-10 V
// enable = 1 -> Range 0-20 V
void AD57X1::setReferenceInputRange(bool enableCompensation) {
  this->controlRegister = (this->controlRegister & ~(0b1111 << this->LINEARITY_COMPENSATION_REGISTER)) | ((enableCompensation ? this->REFERENCE_RANGE_20V : this->REFERENCE_RANGE_10V) << this->LINEARITY_COMPENSATION_REGISTER);
}

void AD57X1::begin() {
  pinMode(this->cs_pin, OUTPUT);
  digitalWrite(this->cs_pin, HIGH);

  if (this->ldac_pin >= 0) {
    pinMode(this->ldac_pin, OUTPUT);
    digitalWrite(this->ldac_pin, LOW);
  }
  //this->reset();
}

