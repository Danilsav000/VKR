#ifndef _MYCYCLE_H
#define _MYCYCLE_H

#define MS_500       500
#define MS_01S      1000
#define MS_02S      2000
#define MS_05S      5000
#define MS_10S     10000
#define MS_13S     13000
#define MS_17S     17000
#define MS_20S     20000
#define MS_30S     30000
#define MS_01M     60000
#define MS_03M    180000
#define MS_05M    300000
#define MS_01H   3600000
#define MS_06H  21600000
#define MS_12H  43200000
#define MS_01D  86400000

class myCycle {
  private:
    bool          _go;
    bool          _active;
    unsigned long _start;
    unsigned long _period;
  
  public:
    myCycle(unsigned long per, bool act);
    void reInit(unsigned long per, bool act);
    void reStart();
    bool check();
    bool go();
    void clear();
    
    // active
    bool active();
    void setActive(bool act);
    // period
    unsigned long period();
    void setPeriod(unsigned long per);
}; // class myCycle

#endif // _MYCYCLE_H