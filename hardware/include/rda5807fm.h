#ifndef RDA5807_H
#define RDA5807_H

struct RadioState {
	bool isEnabled;
	bool isMuted;
	bool isSoftMute;
	bool isMonoForced;
	bool isBassBoost;
	char volume;
	char sensitivity;

	bool isTuning;
	bool tuningComplete;
	bool tuningError;
	bool isTunedToChannel;
	bool isStereo;
	float frequency;
	char signalStrength;

	bool hasRdsData;
	bool hasRdsBlockE;
	char rdsBlockErrors;
	unsigned short rdsBlockA;
	unsigned short rdsBlockB;
	unsigned short rdsBlockC;
	unsigned short rdsBlockD;
	unsigned short rdsBlockE;

	bool hasStationName;
	char stationName[9];
};

class RDA5807_fm {
	public:
		RDA5807_fm();
		bool init();
		void setVolume(char volume);
		void setMute(bool mute);
		void setSoftMute(bool softMute);
		void setForceMono(bool forceMono);
		void setBassBoost(bool bassBoost);
		void setEnable(bool enable);
		void seekUp();
		void seekDown();
		void tuneTo(float frequency);
		void updateStatus();
		float getFrequency();
		const RadioState& getRadioState();

	private:
		unsigned short m_regs[16];
		char m_rdsStationName[8];

		void resetStation();
		void sendRegister(char reg);
		void decodeRdsMessage();
		RadioState m_radio_state;
};

#endif
