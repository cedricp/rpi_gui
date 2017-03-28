#include "rda5807fm.h"

#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"

// Sequential access
#define I2C_SEQ   0x10
// Indexed access
#define I2C_INDEX 0x11

#define RADIO_REG_2 0x02
#define RADIO_REG_3 0x03
#define RADIO_REG_4 0x04
#define RADIO_REG_5 0x05
#define RADIO_REG_6 0x06
#define RADIO_REG_7 0x07

#define RADIO_REG_A 0x0A
#define RADIO_REG_B 0x0B
#define RADIO_REG_C 0x0C
#define RADIO_REG_D 0x0D
#define RADIO_REG_E 0x0E
#define RADIO_REG_F 0x0F

// Register 2 bits
#define R2_AUDIO_OUTPUT   0x8000
#define R2_MUTE_DISABLE   0x4000
#define R2_MONO_SELECT    0x2000
#define R2_BASS_BOOST     0x1000
#define R2_CLOCK_CAL      0x0800
#define R2_RCLK_DIRECT    0x0400
#define R2_SEEK_UP        0x0200
#define R2_SEEK_ENABLE    0x0100
#define R2_SEEK_NO_WRAP   0x0080
#define R2_CLOCK_MODE     0x0070
#define R2_CLOCK_MODE_32K 0x0000
#define R2_CLOCK_MODE_12M 0x0010
#define R2_CLOCK_MODE_13M 0x0020
#define R2_CLOCK_MODE_19M 0X0030
#define R2_CLOCK_MODE_24M 0X0050
#define R2_CLOCK_MODE_26M 0x0060
#define R2_CLOCK_MODE_38M 0x0070
#define R2_RDS_ENABLE     0x0008
#define R2_NEW_METHOD     0x0004
#define R2_SOFT_RESET     0x0002
#define R2_ENABLE         0x0001

// Register 3 bits
#define R3_CHANNEL         0xFFC0
#define R3_DIRECT_MODE     0x0020
#define R3_TUNE_ENABLE     0x0010
#define R3_BAND            0x000C
#define R3_BAND_US_EU      0x0000
#define R3_BAND_JP         0x0004
#define R3_BAND_WORLD      0x0008
#define R3_BAND_E_EU       0x000C
#define R3_CHAN_SPACING    0x0003
#define R3_CHAN_SPACE_100K 0x0000
#define R3_CHAN_SPACE_200K 0x0001
#define R3_CHAN_SPACE_50K  0x0002
#define R3_CHAN_SPACE_25K  0x0003

// Register 4 bits
#define R4_SEEK_TUNE_IE     0x4000
#define R4_DE_EMPHASIS_50   0x0800
#define R4_SOFT_MUTE_ENABLE 0x0200
#define R4_AFC_DISABLE      0x0100
#define R4_I2S_ENABLE       0x0040
#define R4_GPIO3            0x0030
#define R4_GPIO2            0x000C
#define R4_GPIO1            0x0003

// Register 5 bits
#define R5_INTERRUPT_MODE 0x8000
#define R5_SNR_THRESHOLD  0x0800
#define R5_LNA_PORT       0x0080
#define R5_VOLUME         0x000F

#define RA_RDS_READY     0x8000
#define RA_TUNE_COMPLETE 0x4000
#define RA_SEEK_FAIL     0x2000
#define RA_RDS_SYNC      0x1000
#define RA_RDS_BLK_E     0x0800
#define RA_STEREO        0x0400
#define RA_CHANNEL       0x03FF

#define RB_RSSI        0xFE00
#define RB_IS_STATION  0x0100
#define RB_RDS_BLOCK_E 0x0010
#define RB_RDS_ERR     0x000F

#define RDS_GROUP     0xF800
#define RDS_GROUP_A0  0x0000
#define RDS_GROUP_A2  0x2000
#define RDS_GROUP_A4  0x4000
#define RDS_GROUP_A10 0xA000
#define RDS_GROUP_B0  0x0800

RDA5807_fm::RDA5807_fm() {
}


bool RDA5807_fm::init() {
	for (int i = 0; i < 16; i ++) {
		m_regs[i] = 0x0000;
	}

	m_radio_state.isEnabled    = true;
	m_radio_state.isMuted      = false;
	m_radio_state.isSoftMute   = false;
	m_radio_state.isMonoForced = false;
	m_radio_state.isBassBoost  = false;
	m_radio_state.volume       = 5;
	m_radio_state.sensitivity  = 0;

	m_regs[RADIO_REG_2] = R2_SOFT_RESET | R2_ENABLE;
	m_regs[RADIO_REG_3] = R3_BAND_US_EU | R3_CHAN_SPACE_100K;
	m_regs[RADIO_REG_4] = 2048;
	m_regs[RADIO_REG_5] = R5_INTERRUPT_MODE | R5_SNR_THRESHOLD | R5_LNA_PORT | m_radio_state.volume;
	m_regs[RADIO_REG_6] = 0x0000;
	m_regs[RADIO_REG_7] = 0x0000;

	for (char i = RADIO_REG_2; i <= RADIO_REG_7; i ++) {
		sendRegister(i);
	}
	m_regs[RADIO_REG_2] = R2_AUDIO_OUTPUT | R2_MUTE_DISABLE | R2_RDS_ENABLE | R2_NEW_METHOD | R2_ENABLE;
	sendRegister(RADIO_REG_2);

	m_regs[RADIO_REG_3] |= R3_TUNE_ENABLE;
	sendRegister(RADIO_REG_3);

	return true;
}


void RDA5807_fm::setVolume(char volume) {
	m_radio_state.volume = volume;
	m_regs[RADIO_REG_5] = (m_regs[RADIO_REG_5] & 0xFFF0) | (volume & 0x0F);
	sendRegister(RADIO_REG_5);
}


void RDA5807_fm::setMute(bool mute) {
	m_radio_state.isMuted = mute;
	if (mute) {
		m_regs[RADIO_REG_2] &= (~R2_MUTE_DISABLE);
	} else {
		m_regs[RADIO_REG_2] |= R2_MUTE_DISABLE;
	}
	sendRegister(RADIO_REG_2);
}


void RDA5807_fm::setSoftMute(bool softMute) {
	m_radio_state.isSoftMute = softMute;
	if (softMute) {
		m_regs[RADIO_REG_4] |= R4_SOFT_MUTE_ENABLE;
	} else {
		m_regs[RADIO_REG_4] &= (~R4_SOFT_MUTE_ENABLE);
	}
	sendRegister(RADIO_REG_4);
}


void RDA5807_fm::setForceMono(bool forceMono) {
	m_radio_state.isMonoForced = forceMono;
	if (forceMono) {
		m_regs[RADIO_REG_2] |= R2_MONO_SELECT;
	} else {
		m_regs[RADIO_REG_2] &= (~R2_MONO_SELECT);
	}
	sendRegister(RADIO_REG_2);
}


void RDA5807_fm::setBassBoost(bool bassBoost) {
	m_radio_state.isBassBoost = bassBoost;
	if (bassBoost) {
		m_regs[RADIO_REG_2] |= R2_BASS_BOOST;
	} else {
		m_regs[RADIO_REG_2] &= (~R2_BASS_BOOST);
	}
	sendRegister(RADIO_REG_2);
}


void RDA5807_fm::setEnable(bool enable) {
	m_radio_state.isEnabled = enable;
	if (enable) {
		m_regs[RADIO_REG_2] |= R2_ENABLE;
	} else {
		m_regs[RADIO_REG_2] &= (~R2_ENABLE);
	}
	sendRegister(RADIO_REG_2);
}


void RDA5807_fm::seekUp() {
	resetStation();
	m_regs[RADIO_REG_2] |= R2_SEEK_UP | R2_SEEK_ENABLE;
	sendRegister(RADIO_REG_2);
}


void RDA5807_fm::seekDown() {
	resetStation();
	m_regs[RADIO_REG_2] &= (~R2_SEEK_UP);
	m_regs[RADIO_REG_2] |= R2_SEEK_ENABLE;
	sendRegister(RADIO_REG_2);
}


/**
 * Tune to the given frequency in MHz.
 */
void RDA5807_fm::tuneTo(float frequency) {
	unsigned short channelSpacing = m_regs[RADIO_REG_3] & R3_CHAN_SPACING;
	unsigned short band = m_regs[RADIO_REG_3] & R3_BAND;

	// Subtract frequency base depending on current band.
	switch (band) {
		case R3_BAND_US_EU:
			frequency -= 87;
			break;
		case R3_BAND_JP:
		case R3_BAND_WORLD:
			frequency -= 76;
			break;
		case R3_BAND_E_EU:
			frequency -= 65;
			break;
	}

	// Find channel number depending on current channel spacing.
	unsigned short channel;
	switch (channelSpacing) {
		case R3_CHAN_SPACE_25K:
			channel = frequency / 0.025f;
			break;
		case R3_CHAN_SPACE_50K:
			channel = frequency / 0.05f;
			break;
		case R3_CHAN_SPACE_100K:
			channel = frequency / 0.1f;
			break;
		case R3_CHAN_SPACE_200K:
			channel = frequency / 0.2f;
			break;
	}

	resetStation();
	m_regs[RADIO_REG_3] &= (~R3_TUNE_ENABLE);
	sendRegister(RADIO_REG_3);
	m_regs[RADIO_REG_3] |= ((channel << 6) & R3_CHANNEL) | R3_TUNE_ENABLE;
	sendRegister(RADIO_REG_3);
}


void RDA5807_fm::resetStation() {
	m_radio_state.isTuning = true;
	m_radio_state.isTunedToChannel = false;
	m_radio_state.hasStationName = false;
	for (char i = 0; i < 8; i ++) {
		m_radio_state.stationName[i] = ' ';
	}
}


void RDA5807_fm::updateStatus() {
//	Wire.requestFrom(I2C_SEQ, (6 * 2) );
//	for (int i = RADIO_REG_A; i <= RADIO_REG_F; i ++) {
//		m_regs[i] = (Wire.read() << 8) + Wire.read();
//	}
//	Wire.endTransmission();

	int fd = wiringPiI2CSetup(I2C_INDEX);
	for (int i = RADIO_REG_A; i <= RADIO_REG_F; ++i) {
		m_regs[i] = wiringPiI2CReadReg16(fd, i);
	}
	close(fd);

	// Toggle RDS enable flag when new data is received to wait for new data.
	if (m_regs[RADIO_REG_A] & RA_RDS_READY) {
		m_regs[RADIO_REG_2] &= (~R2_RDS_ENABLE);
		sendRegister(RADIO_REG_2);
		m_regs[RADIO_REG_2] |= R2_RDS_ENABLE;
		sendRegister(RADIO_REG_2);
	}

	// When tuned disable tuning and stop seeking.
	if (m_radio_state.isTuning && (m_regs[RADIO_REG_A] & RA_TUNE_COMPLETE)) {
		m_regs[RADIO_REG_3] &= (~R3_TUNE_ENABLE);
		sendRegister(RADIO_REG_3);
		m_regs[RADIO_REG_2] &= (~R2_SEEK_ENABLE);
		sendRegister(RADIO_REG_2);
	}

	// Transform raw data into radioState.
	m_radio_state.hasRdsData       = m_regs[RADIO_REG_A] & RA_RDS_READY;
	m_radio_state.isTuning         = !(m_regs[RADIO_REG_A] & RA_TUNE_COMPLETE);
	m_radio_state.tuningError      = m_regs[RADIO_REG_A] & RA_SEEK_FAIL;
	m_radio_state.hasRdsBlockE     = m_regs[RADIO_REG_A] & RA_RDS_BLK_E;
	m_radio_state.isStereo         = m_regs[RADIO_REG_A] & RA_STEREO;
	m_radio_state.frequency        = getFrequency();
	m_radio_state.signalStrength   = (m_regs[RADIO_REG_B] & RB_RSSI) >> 9;
	m_radio_state.isTunedToChannel = m_regs[RADIO_REG_B] & RB_IS_STATION;
	if (m_radio_state.hasRdsData) {
		m_radio_state.rdsBlockErrors   = m_regs[RADIO_REG_B] & RB_RDS_ERR;
		if (!(m_regs[RADIO_REG_B] & RB_RDS_BLOCK_E)) {
			m_radio_state.rdsBlockA = m_regs[RADIO_REG_C];
			m_radio_state.rdsBlockB = m_regs[RADIO_REG_D];
			m_radio_state.rdsBlockC = m_regs[RADIO_REG_E];
			m_radio_state.rdsBlockD = m_regs[RADIO_REG_F];

			// if (state.rdsBlockErrors == 0) {
			decodeRdsMessage();
			// }
		} else {
			m_radio_state.rdsBlockE = m_regs[RADIO_REG_C];
		}
	}
}


float RDA5807_fm::getFrequency() {
	unsigned short channelSpacing = m_regs[RADIO_REG_3] & R3_CHAN_SPACING;
	unsigned short band = m_regs[RADIO_REG_3] & R3_BAND;
	unsigned short channel = m_regs[RADIO_REG_A] & RA_CHANNEL;

	// Subtract frequency base depending on current band.
	float freq;
	switch (band) {
		case R3_BAND_US_EU:
			freq = 87.0f;
			break;
		case R3_BAND_JP:
		case R3_BAND_WORLD:
			freq = 76.0f;
			break;
		case R3_BAND_E_EU:
			freq = 65.0f;
			break;
	}

		// Add frequency offset depending on channel spacing.
	switch (channelSpacing) {
		case R3_CHAN_SPACE_25K:
			return freq + (channel * 0.025f);
		case R3_CHAN_SPACE_50K:
			return freq + (channel * 0.05f);
		case R3_CHAN_SPACE_100K:
			return freq + (channel * 0.1f);
		case R3_CHAN_SPACE_200K:
			return freq + (channel * 0.2f);
		default:
			return 0;
	}
}


void RDA5807_fm::decodeRdsMessage() {
	switch (m_radio_state.rdsBlockB & RDS_GROUP) {
		case RDS_GROUP_A0:
		case RDS_GROUP_B0:
			char offset = (m_radio_state.rdsBlockB & 0x03) << 1;
			char c1 = (char)(m_radio_state.rdsBlockD >> 8);
			char c2 = (char)(m_radio_state.rdsBlockD & 0xFF);

			// Copy station name byte only if received it twice in a row...
			if (m_rdsStationName[offset] == c1) {
				m_radio_state.stationName[offset] = c1;
				m_radio_state.hasStationName = true;
			} else {
				m_rdsStationName[offset] = c1;
			}

			if (m_rdsStationName[offset + 1] == c2) {
				m_radio_state.stationName[offset + 1] = c2;
				m_radio_state.hasStationName = true;
			} else {
				m_rdsStationName[offset + 1] = c2;
			}

			break;
	}
}

void RDA5807_fm::sendRegister(char reg) {
//	Wire.beginTransmission(I2C_INDEX);
//	Wire.write(reg);
//	Wire.write(m_regs[reg] >> 8);
//	Wire.write(m_regs[reg] & 0xFF);
//	Wire.endTransmission();
	int fd  = wiringPiI2CSetup(I2C_INDEX);
	wiringPiI2CWriteReg16 (fd, reg, m_regs[reg]);
	close(fd);
}

const RDA5807_fm::RadioState& RDA5807_fm::getRadioState()
{
	return m_radio_state;
}
