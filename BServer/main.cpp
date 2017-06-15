#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

const short recv_multicast_port = 9988;
const short send_multicast_port = 9989;

class mserver
{
public:
	mserver(boost::asio::io_service& io_service,
		//const boost::asio::ip::address& listen_address,
		const boost::asio::ip::address& multicast_address)
		: socket_(io_service), send_endpoint_(multicast_address, send_multicast_port), mcount(0)
	{
		// Create the socket so that multiple may be bound to the same address.
		boost::asio::ip::udp::endpoint listen_endpoint(
			boost::asio::ip::udp::v4(), recv_multicast_port);
		socket_.open(listen_endpoint.protocol());
		socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket_.bind(listen_endpoint);

		// Join the multicast group.
		socket_.set_option(
			boost::asio::ip::multicast::join_group(multicast_address));

		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), recv_endpoint_,
			boost::bind(&mserver::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_send_to(const boost::system::error_code& error)
	{
		if (!error)
		{
			mcount++;
			std::cout<<mcount << std::endl;
		}
	}

	void handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error)
		{
			socket_.async_send_to(
				boost::asio::buffer(data_, bytes_recvd), send_endpoint_,
				boost::bind(&mserver::handle_send_to, this,
					boost::asio::placeholders::error));

			socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), recv_endpoint_,
				boost::bind(&mserver::handle_receive_from, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

private:
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint recv_endpoint_;
	boost::asio::ip::udp::endpoint send_endpoint_;
	enum { max_length = 32768 };
	char data_[max_length];

	long long mcount;
};

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;
		mserver r(io_service,
			boost::asio::ip::address::from_string("229.0.0.4"));
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
