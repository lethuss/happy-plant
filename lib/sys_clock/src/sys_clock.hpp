


class Sys_clock {

private:
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
	unsigned long old;

	void resolve_seconds();

public:
	Sys_clock();
	int get_year();
	int get_month();
	int get_day();
	int get_hour();
	int get_minute();
	int get_second();
	void start();
	void run();
	void set_time(short year_p, short month_p, short day_p, short hour_p, short minute_p, short second_p);
	void set_second(short second_p);
	void set_minute(short minute_p);
	void set_hour(short hour_p);
	void set_day(short day_p);

};
