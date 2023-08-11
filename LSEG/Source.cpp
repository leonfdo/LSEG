#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>

using namespace std;

struct sell_inf {
	double flow_price;
	int flow_quantity;
	string flow_id ;

	sell_inf() {
		flow_price = 0;
		flow_quantity = 0;
		flow_id = "";
	}

	sell_inf(double fp, int fq, string fi) {
		flow_price = fp;
		flow_quantity = fq;
		flow_id = fi;
	}

	bool operator()(const sell_inf& a, const sell_inf& b) {
		return a.flow_price > b.flow_price;
	}
};


struct buy_inf {
	double flow_price;
	int flow_quantity;
	string flow_id;

	buy_inf() {
		flow_price = 0;
		flow_quantity = 0;
		flow_id = "";
	}

	buy_inf(double fp, int fq, string fi) {
		flow_price = fp;
		flow_quantity = fq;
		flow_id = fi;
	}

	bool operator()(const buy_inf& a, const buy_inf& b) {
		return a.flow_price < b.flow_price;
	}
};



//class Order {
//	string client_order_id;
//	string instrument;
//	int side;
//	double price;
//	int quantity;
//public:
//	Order(const string Client_order_id, string Instrument, int Side, double Price, int Quantity) : client_order_id{ Client_order_id }, instrument{ Instrument }, side{ Side }, price{ Price }, quantity{ Quantity }{
//	}
//
//	void disp() {
//		cout << "Client_order_id :" << client_order_id << endl;
//		cout << "Instrument :" << instrument << endl;
//		cout << "Side :" << side << endl;
//		cout << "Price :" << price << endl;
//		cout << "Quantity :" << quantity << endl;
//	}
//};





int main() {


	priority_queue<sell_inf,vector<sell_inf>,sell_inf> sell_queue;
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
	file_location = "C:\\Users\\leonf\\OneDrive\\Desktop\\LSEG\\" + file_name;

	fstream myfile;
	myfile.open(file_location);
	if (myfile.is_open()) {
		while (getline(myfile, data)) {
			stringstream line(data);
			getline(line, Client_order_id, ',');
			getline(line, Instrument, ',');
			getline(line, side, ',');
			Side = stoi(side);
			getline(line, price, ',');
			Price = stod(price);
			getline(line, quantity, ',');
			Quantity = stoi(quantity);

			//Order flower_sale(Client_order_id, Instrument, Side, Price, Quantity);
			//sell_inf sell_row(Price, Quantity, Client_order_id);
			//sell_queue.push(sell_row);
			//flower_sale.disp();

			if (Side == 2) {
				sell_inf sell_row(Price, Quantity, Client_order_id);
				sell_queue.push(sell_row);
			}
			else {
				buy_inf sell_row(Price, Quantity, Client_order_id);
				buy_queue.push(sell_row);
			}

		}

	}
	else {
		cout << "Could not open the file\n";
	}

	cout << "the sell book \n" << endl;

	while (!sell_queue.empty()) {
		cout << sell_queue.top().flow_price<<"    " << sell_queue.top().flow_id << endl;
		sell_queue.pop();
	}

	cout << "\nthe buy book \n" << endl;

	while (!buy_queue.empty()) {
		cout << buy_queue.top().flow_price << "    " << buy_queue.top().flow_id << endl;
		buy_queue.pop();
	}

}