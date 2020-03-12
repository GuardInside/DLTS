/* Ћюбой виртаульный пирибор, наследуемый от vi, должен об€зательно вызвать метод connect
перед первым использованием
*/

#ifndef GPIB_H
#define GPIB_H

#include <atomic>
#include <visatype.h>
#include <visa.h>
#include <string>
#include <sstream>

#define READ_BUFFER_SIZE  64
#define QUANTITY_ZONE     10

using std::string;      using std::stringstream;
using std::atomic_bool;

enum class heatlvl{off, low, mid, high};

class vi
{
    public:
        enum switcher {on, off};
    public:
        vi();
        void connect();
    protected:
        /* «аписать сообщение в очередь */
        ViStatus Write(stringstream &message, ViUInt32*  rcount = static_cast<ViUInt32*>(nullptr));
        ViStatus Write(string const &message, ViUInt32*  rcount = static_cast<ViUInt32*>(nullptr));
        /* ѕрочитать сообщение-строку из очереди */
        void ReadStr(string&);
        /* ѕрочитать сообщение-число из очереди */
        void ReadDigit(double& digit);
        void ReadDigit(int& digit);
    public:
        string          ViName;
        int             gpib;
        atomic_bool     bfvi0k;
    private:
        ViSession       hResMeneger;
        ViSession       hInstrument;
        unsigned char   ViBuffer[READ_BUFFER_SIZE];
};

class vi_generator: public vi
{
    public:
        vi_generator();
        void Reset();
        void ErrorCheck(switcher);
        void Channel(switcher);
        void Pulses(switcher);
        void Amp(double);
        void Bias(double);
        void Apply();
    public:
        atomic_bool is_active;
        unsigned int curr_channel;
        double amp;
        double bias;
        double period;
        double width;
    public:
        double step_amp;
        double begin_amp;
        double end_amp;
    public:
        double step_bias;
        double begin_bias;
        double end_bias;
    private:
        void Period(double period);
        void Width(double width);
    private:
        constexpr static double K = 2.3;
};

class zone
{
    public:
        unsigned int P = 50;
        unsigned int I = 20;
        unsigned int D = 0;
        unsigned int upper_boundary = 0;
        heatlvl range = heatlvl::off;
};

class vi_thermostat: public vi
{
    public:
        vi_thermostat();
        void ApplyZones();
        void ApplySetPoint();
        void SetPoint(double);
        void CurrentTemp(double*);
        void CurrentHeatPercent(double*);
        void CurrentSetPoint(double*);
    public:
        void SwitchHeater(switcher);
    public:
        bool range_is_correct();
    public:
        zone   table[QUANTITY_ZONE];
        double BeginPoint;
        double EndPoint;
        double TempStep;
        double TempDisp;
    private:
        heatlvl curr_heatlvl();
};

extern vi_generator Generator;
extern vi_thermostat Thermostat;

#endif
