// reading a text file
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <bitset> 
using namespace std;

int main() {

	const double clock = 0.5;

	string line;
	vector<uint16_t> a, b, r;
	double number;
	ifstream myfile("LSL.txt");

	if (myfile.is_open())
	{

		// Header Line
		getline(myfile, line);
		double targettime = clock / 2;
		uint16_t lasta = 255, lastb = 255;

		cout << "Testing..." << endl;

		while (lasta > 0 || lastb > 0)
		{
			getline(myfile, line);

			double time = 0;
			size_t idx = 0, newposition = 0;
			uint16_t a_buff = 0, b_buff = 0, r_buff = 0;

			// time
			time = stod(&line[newposition], &idx);
			newposition += idx;
			if (time > targettime)
			{
				targettime += clock;

				// a
				for (size_t aindex = 0; aindex < 8; aindex++)
				{
					number = stod(&line[newposition], &idx);
					newposition += idx;
					if (number > 6) a_buff |= 1 << aindex;
				}

				// b
				for (size_t bindex = 0; bindex < 8; bindex++)
				{
					number = stod(&line[newposition], &idx);
					newposition += idx;
					if (number > 6) b_buff |= 1 << bindex;
				}

				// r
				for (size_t rindex = 0; rindex < 8; rindex++)
				{
					number = stod(&line[newposition], &idx);
					newposition += idx;
					if (number > 6) r_buff |= 1 << rindex;
				}

				if (((((a_buff ^ 255) << 1) & 255) | (((a_buff ^ 255) >> 7) & 255))/*& (b_buff ^ 255))*/ != r_buff)
				{
					cout << "ERROR:" << endl;
					cout << "A: " << a_buff << endl;
					cout << "B: " << b_buff << endl;
					cout << "R: " << r_buff << endl;

					bitset<16> abit(((a_buff ^ 255) << 1));
					bitset<16> bbit(((a_buff ^ 255) >> 7));
					bitset<16> rbit(r_buff);

					cout << "A: " << abit << endl;
					cout << "B: " << bbit << endl;
					cout << "R: " << rbit << endl;

					cout << endl;
					break;
				}
				else
				{
					cout << "OK:" << endl;
					cout << "A: " << a_buff << endl;
					cout << "B: " << b_buff << endl;
					cout << "R: " << r_buff << endl;
					cout << endl;
				}

				lasta = a_buff;
				lastb = b_buff;

				/*a.push_back(a_buff);
				b.push_back(b_buff);
				r.push_back(r_buff);

				

				cout << a_buff << endl;
				cout << b_buff << endl;
				cout << r_buff << endl;
				cout << endl;*/
			}

		}

		cout << "Testing Complete" << endl;
		
		myfile.close();
	}

	else cout << "Unable to open file";

	return 0;
}