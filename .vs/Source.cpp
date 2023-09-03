#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
#include <chrono>
#include <iomanip>


using namespace std;



class order {
protected:
	static int nu_orders;
	double flow_price;
	int flow_quantity;
	string flow_id;
	int ID = 0;
public:
	order() {
		flow_price = 0;
		flow_quantity = 0;
		flow_id = "";
	}
	//initiating order attributes
	order(double fp, int fq, string fi) {
		flow_price = fp;
		flow_quantity = fq;
		flow_id = fi;
	}
	//for the unique orderID
	string get_id() const {
		return "ord" + to_string(ID);
	}

	static string  get_static_id() {
		order::nu_orders += 1;
		return "ord" + to_string(order::nu_orders);
	}

	void set_id() {
		order::nu_orders += 1;
		ID = order::nu_orders;
	}

	double get_price() const {
		return flow_price;
	}

	int get_quantity() const {
		return flow_quantity;
	}

	string get_flowid() const {
		return flow_id;
	}

	//Update order quantity after PFills
	void update(const int a) {
		flow_quantity = flow_quantity - a;
	}


};

order a;

int order::nu_orders = 0;

class sell_inf :public order {
	//int id;
public:
	sell_inf() :order() {}
	//initializing sell order attributes
	sell_inf(double fp, int fq, string fi) :order(fp, fq, fi) {
		//id = nu_orders;
	}

	//to sort by descending order of price
	bool operator()(const sell_inf& a, const sell_inf& b) {
		return a.flow_price > b.flow_price;
	}

};


class buy_inf :public order {
	//int id;
public:
	buy_inf() :order() {};
	//initializing buy order attributes
	buy_inf(double fp, int fq, string fi) :order(fp, fq, fi) {
		//id = nu_orders;
	}

	//to sort by ascending order of price
	bool operator()(const buy_inf& a, const buy_inf& b) {
		return a.flow_price < b.flow_price;
	}

};


//Function to get current time
string Get_time() {
	// Get the current time
	auto now = chrono::system_clock::now();

	// Adjust the current time to Sri Lanka Standard Time
	auto local_time = now; //+ offset;

	// Convert the local time to a time_t object
	time_t t = chrono::system_clock::to_time_t(local_time);

	// Get milliseconds
	auto ms = chrono::duration_cast<chrono::milliseconds>(local_time.time_since_epoch()) % 1000;

	// Convert the time to a tm structure in the local time zone
	tm tm_local;

#ifdef _WIN32
	localtime_s(&tm_local, &t); // Use localtime_s on Windows
#else
	localtime_r(&t, &tm_local); // Use localtime_r on non-Windows platforms
#endif

	stringstream timestamp;
	timestamp << put_time(&tm_local, "%Y%m%d-%H%M%S") << "." << setw(3) << setfill('0') << ms.count();
	return timestamp.str();
}

//Defining the instrument books
typedef map<string, pair<priority_queue<sell_inf, vector<sell_inf>, sell_inf>, priority_queue<buy_inf, vector<buy_inf>, buy_inf>>> inst_book;

struct op_book {
	inst_book book;

	//Function to match orders
	template <class t> void check(string name, string side, ofstream& executionReport, t row, bool not_recur = true) {
		string time_str = Get_time();
		if (book[name].first.empty() || book[name].second.empty()) {
			if (not_recur) {
				executionReport << row.get_id() << "," << row.get_flowid() << "," << name << "," << side << ",New," << row.get_quantity() << "," << fixed << setprecision(2) << row.get_price() << "," << " " << "," << time_str << ",\n";
			}
			else { return; }
		}
		else if (book[name].first.top().get_price() <= book[name].second.top().get_price()) {
			if (book[name].first.top().get_quantity() < book[name].second.top().get_quantity()) {
				//book[name].first.pop();
				//book[name].second.top().update();
				buy_inf temp = book[name].second.top();
				temp.update(book[name].first.top().get_quantity());
				book[name].second.pop();
				book[name].second.push(temp);
				//delete temp;
				//book[name].second.top().update(book[name].second.top().get_quantity());
				executionReport << book[name].second.top().get_id() << "," << book[name].second.top().get_flowid() << "," << name << ",1" << ",Pfill," << book[name].first.top().get_quantity() << "," << fixed << setprecision(2) << book[name].first.top().get_price() << "," << "" << "," << time_str << ",\n";
				executionReport << book[name].first.top().get_id() << "," << book[name].first.top().get_flowid() << "," << name << ",2" << ",Fill," << book[name].first.top().get_quantity() << ", " << fixed << setprecision(2) << book[name].first.top().get_price() << "," << "" << "," << time_str << ", \n";
				book[name].first.pop();
			}
			else if (book[name].first.top().get_quantity() == book[name].second.top().get_quantity()) {
				executionReport << book[name].second.top().get_id() << "," << book[name].second.top().get_flowid() << "," << name << ",1" << ",Fill," << book[name].second.top().get_quantity() << "," << fixed << setprecision(2) << book[name].first.top().get_price() << "," << "" << "," << time_str << ",\n";
				executionReport << book[name].first.top().get_id() << "," << book[name].first.top().get_flowid() << "," << name << ",2" << ",Fill," << book[name].first.top().get_quantity() << "," << fixed << setprecision(2) << book[name].first.top().get_price() << "," << "" << "," << time_str << ",\n";
				book[name].first.pop();
				book[name].second.pop();
			}
			else {
				sell_inf temp = book[name].first.top();
				temp.update(book[name].second.top().get_quantity());
				book[name].first.pop();
				book[name].first.push(temp);
				executionReport << book[name].first.top().get_id() << "," << book[name].first.top().get_flowid() << "," << name << ",2" << ",Pfill," << book[name].second.top().get_quantity() << ", " << fixed << setprecision(2) << book[name].second.top().get_price() << "," << "" << "," << time_str << ", \n";
				executionReport << book[name].second.top().get_id() << "," << book[name].second.top().get_flowid() << "," << name << ",1" << ",Fill," << book[name].second.top().get_quantity() << "," << fixed << setprecision(2) << book[name].second.top().get_price() << "," << "" << "," << time_str << ",\n";
				book[name].second.pop();
				check<sell_inf>(name, side, executionReport, temp, false);
			}

		}
		else {
			if (not_recur) {
				executionReport << row.get_id() << "," << row.get_flowid() << "," << name << "," << side << ",New," << row.get_quantity() << "," << fixed << setprecision(2) << row.get_price() << "," << " " << "," << time_str << ",\n";
			}
			else { return; }
		}
	}
};


int main() {
	inst_book book;
	op_book instrument_books;
	instrument_books.book = book;

	//Priority queues for buy and sell orders
	priority_queue<sell_inf, vector<sell_inf>, sell_inf> sell_queue;
	priority_queue<buy_inf, vector<buy_inf>, buy_inf> buy_queue;

	string file_location;
	string data;
	string file_name;

	string Client_order_id;
	string Instrument;
	string side;
	int Side;
	string price;
	double Price;
	string quantity;
	int Quantity;



	cout << "Enter the file name:";
	cin >> file_name;
	//Reading file location
	file_location = "Orders\\" + file_name;



	//Writing file destination
	ofstream executionReport("Reports\\execution_report.csv");
	// Exit the program with an error code if opened
	if (!executionReport.is_open()) {
		cout << "The given path is wrong , Please enter the correct path" << "\n";
		return 1;
	}
	//Header of the execution report
	executionReport << "OrderID,Client_order_id,Instrument,Side,Execution_Status,Quantity,Price,Reason_If_Rejected,Transaction_time\n";



	//The list of valid instruments
	vector<string> validInstruments = { "Rose", "Lavender","Lotus","Tulip","Orchid" };

	//Reading the file
	fstream myfile;
	myfile.open(file_location);
	if (myfile.is_open()) {
		getline(myfile, data); //discarding the first line
		while (getline(myfile, data)) {
			stringstream line(data);
			getline(line, Client_order_id, ',');
			getline(line, Instrument, ',');
			getline(line, side, ',');
			Side = stoi(side);
			getline(line, quantity, ',');
			Quantity = stoi(quantity);
			getline(line, price, ',');
			Price = stod(price);

			//Getting the current time
			string time_str = Get_time();


			//Order flower_sale(Client_order_id, Instrument, Side, Price, Quantity);
			//sell_inf sell_row(Price, Quantity, Client_order_id);
			//sell_queue.push(sell_row);
			//flower_sale.disp();
			if (Client_order_id.empty() || Instrument.empty() || side.empty() || price.empty() || quantity.empty()) {
				//cout << "Error: Mandotory fields Missing." << "\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << Price << ",Missing Fields," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			else if (find(validInstruments.begin(), validInstruments.end(), Instrument) == validInstruments.end()) {
				//cout << "Error: Invalid instrument '" << Instrument << "'. To Rejection Notice" << "\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << Price << ",Invalid Instrument," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			else if (Side != 1 && Side != 2) {
				//cout << "Error: Invalid side '" << Side << "'. Side should be 1 (buy) or 2 (sell)." << "\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << fixed << setprecision(2) << Price << ",Invalid Side," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			// Check if the price is greater than 0
			else if (Price <= 0) {
				//cout << "Error: Invalid price '" << Price << "'. Price should be greater than 0." <<"\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << fixed << setprecision(2) << Price << ",Invalid Price," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}

			// Check if the quantity is a multiple of 10
			else if (Quantity % 10 != 0) {
				//cout << "Error: Invalid quantity '" << Quantity << "'. Quantity should be a multiple of 10." << "\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << fixed << setprecision(2) << Price << ",Invalid Quantity," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}

			// Check if the quantity is within the range (min = 10 max = 1000)
			else if (Quantity < 10 || Quantity > 1000) {
				cout << "Error: Invalid quantity '" << Quantity << "'. Quantity should be in the range of 10 to 1000." << "\n";
				executionReport << order::get_static_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << fixed << setprecision(2) << Price << ",Quantity Out Of Range," << time_str << "\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}


			else if (Side == 2) {
				sell_inf sell_row(Price, Quantity, Client_order_id);
				sell_row.set_id();
				instrument_books.book[Instrument].first.push(sell_row); // Update sell book
				instrument_books.check<sell_inf>(Instrument, side, executionReport, sell_row);
				//if(instrument_books[Instrument].second.top()>=instrument_books[Instrument].first.top())
				//executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << "," << "Executed," << Quantity << "," << Price << ",\n";
			}
			else {
				buy_inf buy_row(Price, Quantity, Client_order_id);
				buy_row.set_id();
				instrument_books.book[Instrument].second.push(buy_row); // Update buy book
				instrument_books.check<buy_inf>(Instrument, side, executionReport, buy_row);
				//executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << "," << "Executed," << Quantity << "," << Price << ",\n";
			}



		}

		// Writing the exchange applications for each instrument to a CSV file after processing all entries.
		cout << "Separate Exchange Application Files" << '\n';

		for (auto& instrument_book : instrument_books.book) {
			//Exchange application file location
			file_location = "Reports\\exchange_";
			const string& instrument = instrument_book.first;
			auto& sell_queue = instrument_book.second.first;
			auto& buy_queue = instrument_book.second.second;
			file_location = file_location + instrument + ".csv";
			cout << file_location << "\n";
			// Open the file
			ofstream exchange(file_location);

			if (exchange.is_open()) {
				exchange << instrument << "\n ";
				exchange << "sell book,,,,Buy Book\n ";
				exchange << "OrderID,Quantity,Price,,Price,Quantity,OrderID\n";

				// Continue writing until both queues are empty
				while (!buy_queue.empty() || !sell_queue.empty()) {
					if (!buy_queue.empty()) {
						exchange << buy_queue.top().get_id() << "," << buy_queue.top().get_quantity() << "," << fixed << setprecision(2) << buy_queue.top().get_price() << ",,";
						buy_queue.pop();
					}
					else {
						exchange << ", , , ,";
					}

					if (!sell_queue.empty()) {
						exchange << sell_queue.top().get_price() << "," << sell_queue.top().get_quantity() << "," << fixed << setprecision(2) << sell_queue.top().get_id() << "\n";
						sell_queue.pop();
					}
					else {
						exchange << ",,\n";
					}
				}

				exchange.close(); // Close the file when finished with this instrument
			}
			else {
				cerr << "The given path is wrong, Please enter the correct path " << file_location << "\n";
			}
		}



	}
	else {
		cout << "The given path is wrong , Please enter the correct path\n";
	}




	executionReport.close();
};