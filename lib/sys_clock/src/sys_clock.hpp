


class Sys_clock {

private:
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;

	long old;

	void run();


public:
	Sys_clock();
	int get_year();
	int get_month();
	int get_day();
	int get_hour();
	int get_minute();
	int get_second();
	void start();
};
