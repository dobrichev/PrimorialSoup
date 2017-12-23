#include <string>
#define MAX 10000 // for strings

//using namespace std;
//-------------------------------------------------------------
class BigInteger
{
private:
	std::string number;
	bool sign;
public:
	BigInteger(); // empty constructor initializes zero
	BigInteger(const std::string& s); // "string" constructor
	BigInteger(const std::string& s, bool sin); // "string" constructor
	BigInteger(const char* s); // "12345" constructor
	BigInteger(int n); // "int" constructor
	BigInteger(unsigned int n); // "int" constructor
	BigInteger(long double ld); // "long long double" constructor
	void setNumber(const std::string& s);
	const std::string& getNumber() const; // retrieves the number
	void setSign(bool s);
	const bool& getSign() const;
	BigInteger absolute() const; // returns the absolute value
	void operator = (const BigInteger& b);
	bool operator == (const BigInteger& b) const;
	bool operator != (const BigInteger& b) const;
	bool operator > (const BigInteger& b) const;
	bool operator < (const BigInteger& b) const;
	bool operator >= (const BigInteger& b) const;
	bool operator <= (const BigInteger& b) const;
	BigInteger& operator ++(); // prefix
	BigInteger  operator ++(int); // postfix
	BigInteger& operator --(); // prefix
	BigInteger  operator --(int); // postfix
	BigInteger operator + (const BigInteger& b) const;
	BigInteger operator + (int n) const;
	BigInteger operator - (const BigInteger& b) const;
	BigInteger operator - (int n) const;
	BigInteger operator * (const BigInteger& b) const;
	BigInteger operator / (const BigInteger& b) const;
	BigInteger operator % (const BigInteger& b) const;
	BigInteger& operator += (const BigInteger& b);
	BigInteger& operator -= (const BigInteger& b);
	BigInteger& operator *= (const BigInteger& b);
	BigInteger& operator /= (const BigInteger& b);
	BigInteger& operator %= (const BigInteger& b);
	BigInteger& operator [] (int n);
	BigInteger operator -(); // unary minus sign
	operator std::string() const; // for conversion from BigInteger to string
	operator long double() const;
	static std::pair<std::string, long long> divide(const std::string& n, long long den);
private:
	bool equals(const BigInteger& n1, const BigInteger& n2) const;
	bool less(const BigInteger& n1, const BigInteger& n2) const;
	bool greater(const BigInteger& n1, const BigInteger& n2) const;
	std::string add(const std::string& number1, const std::string& number2) const;
	std::string subtract(const std::string& number1, const std::string& number2) const;
	std::string multiply(const std::string& number1, const std::string& number2) const;
	std::string multiply(const std::string& n1, const long long n2) const;
	std::string toString(long long n) const;
	long long toInt(const std::string& s) const;
};

