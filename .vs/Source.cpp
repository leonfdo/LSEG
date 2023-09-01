#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <map>

using namespace std;


class order {
protected:
	static int nu_orders;
	double flow_price;
	int flow_quantity;
	string flow_id;
	int select;
public:
	order() {
		flow_price = 0;
		flow_quantity = 0;
		flow_id = "";
	}

	order(double fp, int fq, string fi, int select = 0) {
		flow_price = fp;
		flow_quantity = fq;
		flow_id = fi;
		select = select;
	}

	static string get_id() {
		nu_orders += 1;
		return "ord" + to_string(nu_orders);
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

	void update(const int a) {
		flow_quantity = flow_quantity - a;
	}


};

order a;

int order::nu_orders = 0;

class sell_inf:public order{
	int id;
public:
	sell_inf():order(){}

	sell_inf(double fp, int fq, string fi):order(fp,fq,fi){
		id = nu_orders;
	}

	bool operator()(const sell_inf& a, const sell_inf& b) {
		return a.flow_price > b.flow_price;
	}

	void display() const{
		cout << flow_price << "        " << flow_quantity << "        " << flow_id << "    "<< id <<endl;
	}
};


class buy_inf :public order{
	int id;
public:
	buy_inf() :order() {};

	buy_inf(double fp, int fq, string fi):order(fp, fq, fi){
		id = nu_orders;
	}

	bool operator()(const buy_inf& a, const buy_inf& b) {
		return a.flow_price < b.flow_price;
	}

	void display() const{
		cout << flow_price << "        " << flow_quantity << "        " << flow_id <<"     "<< id << endl;
	}
};



void print_books(const priority_queue<sell_inf, vector<sell_inf>, sell_inf>& sell_queue, const priority_queue<buy_inf, vector<buy_inf>, buy_inf>& buy_queue) {
	cout << endl;
	cout << "the sell book \n";
	cout << "Price " << "  " << "Quantity " << "  " << "OrderId" << endl;
	priority_queue<sell_inf, vector<sell_inf>, sell_inf> temp_sell_queue = sell_queue; // Create a copy for display
	while (!temp_sell_queue.empty()) {
		temp_sell_queue.top().display();
		temp_sell_queue.pop();
	}

	cout << "\nthe buy book \n";
	cout << "Price " << "  " << "Quantity " << "  " << "OrderId" << endl;
	priority_queue<buy_inf, vector<buy_inf>, buy_inf> temp_buy_queue = buy_queue; // Create a copy for display
	while (!temp_buy_queue.empty()) {
		temp_buy_queue.top().display();
		temp_buy_queue.pop();
	}
	cout << endl;
}

typedef map<string, pair<priority_queue<sell_inf, vector<sell_inf>, sell_inf>, priority_queue<buy_inf, vector<buy_inf>, buy_inf>>> inst_book;

struct op_book {
	inst_book book;

	template <class t> void check(string name,string side,ofstream& executionReport,t row ,bool not_recur=true) {
		if (book[name].first.empty() || book[name].second.empty()) {
			if (not_recur) {
				executionReport << order::get_id() << "," << row.get_flowid() << "," << name << "," << side << ",new," << row.get_quantity() << "," << row.get_price() << ",\n";
			}
			else { return; }
		}
		else if (book[name].first.top().get_price() <= book[name].second.top().get_price()) {
			if (book[name].first.top().get_quantity() < book[name].second.top().get_quantity()) {
				executionReport << order::get_id() << "," << book[name].first.top().get_flowid() << "," << name<< ",2"<<",fill," << book[name].first.top().get_quantity() << ", " << book[name].first.top().get_price() << ", \n";
				//book[name].first.pop();
				//book[name].second.top().update();
				buy_inf temp=book[name].second.top();
				temp.update(book[name].first.top().get_quantity());
				book[name].second.pop();
				book[name].second.push(temp);
				//delete temp;
				//book[name].second.top().update(book[name].second.top().get_quantity());
				executionReport << order::get_id() << "," << book[name].second.top().get_flowid() << "," << name <<",1"<< ",pfill," << book[name].first.top().get_quantity() << "," << book[name].first.top().get_price() << ",\n";
				book[name].first.pop();
			}
			else if(book[name].first.top().get_quantity() == book[name].second.top().get_quantity()) {
				executionReport << order::get_id() << "," << book[name].first.top().get_flowid() << "," << name << ",2" <<",fill," << book[name].first.top().get_quantity() << "," << book[name].first.top().get_price() << ",\n";
				executionReport << order::get_id() << "," << book[name].second.top().get_flowid() << "," << name << ",1" <<",fill," << book[name].second.top().get_quantity() << "," << book[name].first.top().get_price() << ",\n";
				book[name].first.pop();
				book[name].second.pop();
			}
			else {
				executionReport << order::get_id() << "," << book[name].second.top().get_flowid() << "," << name << ",1" <<",fill," << book[name].second.top().get_quantity() << "," << book[name].second.top().get_price() << ",\n";
				sell_inf temp = book[name].first.top();
				temp.update(book[name].second.top().get_quantity());
				book[name].first.pop();
				book[name].first.push(temp);
				executionReport << order::get_id() << "," << book[name].first.top().get_flowid() << "," << name << ",2" <<",pfill,"  << book[name].second.top().get_quantity() << ", " << book[name].second.top().get_price() << ", \n";
				book[name].second.pop();
				check<sell_inf>(name, side, executionReport, temp,false);
			}

		}
		else {
			if (not_recur) {
				executionReport << order::get_id() << "," << row.get_flowid() << "," << name << "," << side << ",new," << row.get_quantity() << "," << row.get_price() << ",\n";
			}
			else { return; }
		}
	}
};


int main() {
	inst_book book;
	op_book instrument_books;
	instrument_books.book = book;

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
	file_location = "C:\\Users\\leonf\\OneDrive\\Desktop\\LSEG\\Orders\\" + file_name;




	ofstream executionReport("C:\\Users\\leonf\\OneDrive\\Desktop\\LSEG\\Reports\\report.csv");
	if (!executionReport.is_open()) {
		cout << "Error: Failed to create the execution report file." << endl;
		return 1; // Exit the program with an error code
	}

	executionReport << "OrderID,Client_order_id,Instrument,Side,Execution_Status,Quantity,Price,Reason_If_Rejected\n";




	vector<string> validInstruments = { "Rose", "Lavender","Lotus","Tulip","Orchid" };

	fstream myfile;
	myfile.open(file_location);
	if (myfile.is_open()) {
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


			//Order flower_sale(Client_order_id, Instrument, Side, Price, Quantity);
			//sell_inf sell_row(Price, Quantity, Client_order_id);
			//sell_queue.push(sell_row);
			//flower_sale.disp();
			if (Client_order_id.empty() || Instrument.empty() || side.empty() || price.empty() || quantity.empty()) {
				cout << "Error: Mandotory fields Missing." << endl;
				executionReport <<order::get_id() << "," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Missing Fields\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			if (find(validInstruments.begin(), validInstruments.end(), Instrument) == validInstruments.end()) {
				cout << "Error: Invalid instrument '" << Instrument << "'. To Rejection Notice" << endl;
				executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Invalid Instrument\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			if (Side != 1 && Side != 2) {
				cout << "Error: Invalid side '" << Side << "'. Side should be 1 (buy) or 2 (sell)." << endl;
				executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Invali dSide\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}
			// Check if the price is greater than 0
			if (Price <= 0) {
				cout << "Error: Invalid price '" << Price << "'. Price should be greater than 0." << endl;
				executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Invalid Price\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}

			// Check if the quantity is a multiple of 10
			if (Quantity % 10 != 0) {
				cout << "Error: Invalid quantity '" << Quantity << "'. Quantity should be a multiple of 10." << endl;
				executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Invalid Quantity\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}

			// Check if the quantity is within the range (min = 10 max = 1000)
			if (Quantity < 10 || Quantity > 1000) {
				cout << "Error: Invalid quantity '" << Quantity << "'. Quantity should be in the range of 10 to 1000." << endl;
				executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << ",Rejected,," << price << ",Quantity Out Of Range\n";
				continue; // Skip the current iteration and proceed with the next line in the file
			}


			if (Side == 2) {
				sell_inf sell_row(Price, Quantity, Client_order_id);
				instrument_books.book[Instrument].first.push(sell_row); // Update sell book
				instrument_books.check<sell_inf>(Instrument,side,executionReport,sell_row);
				//if(instrument_books[Instrument].second.top()>=instrument_books[Instrument].first.top())
				//executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << "," << "Executed," << Quantity << "," << Price << ",\n";
			}
			else {
				buy_inf buy_row(Price, Quantity, Client_order_id);
				instrument_books.book[Instrument].second.push(buy_row); // Update buy book
				instrument_books.check<buy_inf>(Instrument,side,executionReport,buy_row);
				//executionReport << order::get_id() <<"," << Client_order_id << "," << Instrument << "," << side << "," << "Executed," << Quantity << "," << Price << ",\n";
			}



		}

		// Print the updated books for each instrument after processing all entries


		for (const auto& instrument_book : instrument_books.book) {
			const string& instrument = instrument_book.first;
			auto& sell_queue = instrument_book.second.first;
			auto& buy_queue = instrument_book.second.second;

			cout << instrument << " Book\n";
			print_books(sell_queue, buy_queue);
		}



	}
	else {
		cout << "Could not open the file\n";
	}




	executionReport.close();
}