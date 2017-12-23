#ifndef BIGINTEGER_H
#define BIGINTEGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include "BigInteger.h"
//#define MAX 10000 // for strings

BigInteger::BigInteger() // empty constructor initializes zero
{
	number = "0";
	sign = false;
}
//-------------------------------------------------------------
BigInteger::BigInteger(const std::string& s) // "string" constructor
{
	if( isdigit(s[0]) ) // if not signed
	{
		setNumber(s);
		sign = false; // +ve
	}
	else
	{
		setNumber( s.substr(1) );
		sign = (s[0] == '-');
	}
}
//-------------------------------------------------------------
BigInteger::BigInteger(const std::string& s, bool sin) // "string" constructor
{
	setNumber( s );
	setSign( sin );
}
//-------------------------------------------------------------
BigInteger::BigInteger(const char* s) // "12345" constructor
{
	std::string ss(s);
	setNumber( s );
}
//-------------------------------------------------------------
BigInteger::BigInteger(int n) // "int" constructor
{
	std::stringstream ss;
	std::string s;
	ss << n;
	ss >> s;

	if( isdigit(s[0]) ) // if not signed
	{
		setNumber( s );
		setSign( false ); // +ve
	}
	else
	{
		setNumber( s.substr(1) );
		setSign( s[0] == '-' );
	}
}
//-------------------------------------------------------------
BigInteger::BigInteger(unsigned int ui) : sign(false)// "int" constructor
{
	std::stringstream ss;
	ss << ui;
	ss >> number;
}
//-------------------------------------------------------------
BigInteger::BigInteger(long double ld) // "int" constructor
{
	//1.4774341727612374e+78
	setSign(ld < 0);
	if(ld == 0.) {
		number = "0";
		return;
	}
	std::string s;
	std::stringstream ss;
	ss.precision(17);
	ss << abs(ld);
	ss >> s;
	//cout << s << "\t";
	while(s[0] == '0') s.erase(0);
	int numZeroesToAdd = 0;
	size_t expPos = s.find("e+");
	if(expPos != std::string::npos) {
		numZeroesToAdd = std::stoi(s.substr(expPos + 2));
		s.erase(expPos);
	}
	size_t pointPos = s.find(".");
	if(pointPos != std::string::npos) {
		s.erase(pointPos, 1);
		numZeroesToAdd -= (s.size() - pointPos);
	}
	if(numZeroesToAdd > 0) {
		s.append(std::string(numZeroesToAdd, '0'));
	}
	else if(numZeroesToAdd < 0) {
		s.erase(s.size() + numZeroesToAdd);
	}
	number = s;
	//cout << s << "\n";
}
//-------------------------------------------------------------
void BigInteger::setNumber(const std::string& s)
{
	number = s;
}
//-------------------------------------------------------------
const std::string& BigInteger::getNumber() const // retrieves the number
{
	return number;
}
//-------------------------------------------------------------
void BigInteger::setSign(bool s)
{
	sign = s;
}
//-------------------------------------------------------------
const bool& BigInteger::getSign() const
{
	return sign;
}
//-------------------------------------------------------------
// returns the absolute value
BigInteger BigInteger::absolute() const
{
	return BigInteger( getNumber() ); // +ve by default
}
//-------------------------------------------------------------
void BigInteger::operator = (const BigInteger& b)
{
	setNumber( b.getNumber() );
	setSign( b.getSign() );
}
//-------------------------------------------------------------
bool BigInteger::operator == (const BigInteger& b) const
{
	return equals((*this) , b);
}
//-------------------------------------------------------------
bool BigInteger::operator != (const BigInteger& b) const
{
	return ! equals((*this) , b);
}
//-------------------------------------------------------------
bool BigInteger::operator > (const BigInteger& b) const
{
	return greater((*this) , b);
}
//-------------------------------------------------------------
bool BigInteger::operator < (const BigInteger& b) const
{
	return less((*this) , b);
}
//-------------------------------------------------------------
bool BigInteger::operator >= (const BigInteger& b) const
{
	return equals((*this) , b)
		|| greater((*this), b);
}
//-------------------------------------------------------------
bool BigInteger::operator <= (const BigInteger& b) const
{
	return equals((*this) , b) 
		|| less((*this) , b);
}
//-------------------------------------------------------------
// increments the value, then returns its value
BigInteger& BigInteger::operator ++() // prefix
{
	(*this) = (*this) + 1;
	return (*this);
}
//-------------------------------------------------------------
// returns the value, then increments its value
BigInteger BigInteger::operator ++(int) // postfix
{
	BigInteger before = (*this);

	(*this) = (*this) + 1;

	return before;
}
//-------------------------------------------------------------
// decrements the value, then return it
BigInteger& BigInteger::operator --() // prefix
{
	(*this) = (*this) - 1;
	return (*this);

}
//-------------------------------------------------------------
// return the value, then decrements it
BigInteger BigInteger::operator --(int) // postfix
{	
	BigInteger before = (*this);

	(*this) = (*this) - 1;

	return before;
}
//-------------------------------------------------------------
BigInteger BigInteger::operator + (int b) const
{
	BigInteger tmp(b);
	return (*this) + tmp;
}
//-------------------------------------------------------------
BigInteger BigInteger::operator + (const BigInteger& b) const
{
	BigInteger addition;
	if( getSign() == b.getSign() ) // both +ve or -ve
	{
		addition.setNumber( add(getNumber(), b.getNumber() ) );
		addition.setSign( getSign() );
	}
	else // sign different
	{
		if( absolute() > b.absolute() )
		{
			addition.setNumber( subtract(getNumber(), b.getNumber() ) );
			addition.setSign( getSign() );
		}
		else
		{
			addition.setNumber( subtract(b.getNumber(), getNumber() ) );
			addition.setSign( b.getSign() );
		}
	}
	if(addition.getNumber() == "0") // avoid (-0) problem
		addition.setSign(false);

	return addition;
}
//-------------------------------------------------------------
BigInteger BigInteger::operator - (int b) const
{
	BigInteger tmp(b);
	return (*this) - tmp;
}
//-------------------------------------------------------------
BigInteger BigInteger::operator - (const BigInteger& b) const
{
	BigInteger tmp(b); //a bit stupid
	tmp.setSign( ! tmp.getSign() ); // x - y = x + (-y)
	return (*this) + tmp;
}
//-------------------------------------------------------------
BigInteger BigInteger::operator * (const BigInteger& b) const
{
	BigInteger mul;

	mul.setNumber( multiply(getNumber(), b.getNumber() ) );
	mul.setSign( getSign() != b.getSign() );

	if(mul.getNumber() == "0") // avoid (-0) problem
		mul.setSign(false);

	return mul;
}
//-------------------------------------------------------------
// Warning: Denomerator must be within "long long" size not "BigInteger"
BigInteger BigInteger::operator / (const BigInteger& b) const
{
	long long den = toInt( b.getNumber() );
	BigInteger div;

	div.setNumber( divide(getNumber(), den).first );
	div.setSign( getSign() != b.getSign() );

	if(div.getNumber() == "0") // avoid (-0) problem
		div.setSign(false);

	return div;
}
//-------------------------------------------------------------
// Warning: Denomerator must be within "long long" size not "BigInteger"
BigInteger BigInteger::operator % (const BigInteger& b) const
{
	long long den = toInt( b.getNumber() );

	BigInteger rem;
	long long rem_int = divide(number, den).second;
	rem.setNumber( toString(rem_int) );
	rem.setSign( getSign() != b.getSign() );

	if(rem.getNumber() == "0") // avoid (-0) problem
		rem.setSign(false);

	return rem;
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator += (const BigInteger& b)
{
	(*this) = (*this) + b;
	return (*this);
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator -= (const BigInteger& b)
{
	(*this) = (*this) - b;
	return (*this);
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator *= (const BigInteger& b)
{
	(*this) = (*this) * b;
	return (*this);
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator /= (const BigInteger& b)
{
	(*this) = (*this) / b;
	return (*this);
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator %= (const BigInteger& b)
{
	(*this) = (*this) % b;
	return (*this);
}
//-------------------------------------------------------------
BigInteger& BigInteger::operator [] (int n)
{
	return *(this + (n*sizeof(BigInteger)));
}
//-------------------------------------------------------------
BigInteger BigInteger::operator -() // unary minus sign
{
	BigInteger ret(*this);
	ret.sign = ! sign;
	return ret;
}
//-------------------------------------------------------------
BigInteger::operator std::string() const // for conversion from BigInteger to string
{
	std::string signedString = ( getSign() ) ? "-" : ""; // if +ve, don't print + sign
	signedString += number;
	return signedString;
}
//-------------------------------------------------------------
BigInteger::operator long double() const
{
	long double ret;
	std::stringstream ss;
	ss << "0." << this->number.substr(0, std::min((size_t)25, this->number.length())) << "e+" << this->number.size();
	ss >> ret;
	return ret;
}
//-------------------------------------------------------------

bool BigInteger::equals(const BigInteger& n1, const BigInteger& n2) const
{
	return n1.getNumber() == n2.getNumber()
		&& n1.getSign() == n2.getSign();
}

//-------------------------------------------------------------
bool BigInteger::less(const BigInteger& n1, const BigInteger& n2) const
{
	bool sign1 = n1.getSign();
	bool sign2 = n2.getSign();

	if(sign1 && ! sign2) // if n1 is -ve and n2 is +ve
		return true;

	else if(! sign1 && sign2)
		return false;

	else if(! sign1) // both +ve
	{
		if(n1.getNumber().length() < n2.getNumber().length() )
			return true;
		if(n1.getNumber().length() > n2.getNumber().length() )
			return false;
		return n1.getNumber() < n2.getNumber();
	}
	else // both -ve
	{
		if(n1.getNumber().length() > n2.getNumber().length())
			return true;
		if(n1.getNumber().length() < n2.getNumber().length())
			return false;
		return n1.getNumber().compare( n2.getNumber() ) > 0; // greater with -ve sign is LESS
	}
}
//-------------------------------------------------------------
bool BigInteger::greater(const BigInteger& n1, const BigInteger& n2) const
{
	return ! equals(n1, n2) && ! less(n1, n2);
}

//-------------------------------------------------------------
// adds two strings and returns their sum in as a string
std::string BigInteger::add(const std::string& n1, const std::string& n2) const
{
	std::string number1(n1); //hmmmm
	std::string number2(n2); //hmmmm
	std::string add = (number1.length() > number2.length()) ?  number1 : number2;
	char carry = '0';
	int differenceInLength = abs( (int) (number1.size() - number2.size()) );

	if(number1.size() > number2.size())
		number2.insert(0, differenceInLength, '0'); // put zeros from left

	else// if(number1.size() < number2.size())
		number1.insert(0, differenceInLength, '0');

	for(int i=number1.size()-1; i>=0; --i)
	{
		add[i] = ((carry-'0')+(number1[i]-'0')+(number2[i]-'0')) + '0';

		if(i != 0)
		{	
			if(add[i] > '9')
			{
				add[i] -= 10;
				carry = '1';
			}
			else
				carry = '0';
		}
	}
	if(add[0] > '9')
	{
		add[0]-= 10;
		add.insert(0,1,'1');
	}
	return add;
}

//-------------------------------------------------------------
// subtracts two strings and returns their sum in as a string
std::string BigInteger::subtract(const std::string& n1, const std::string& n2) const
{
	std::string number1(n1); //hmmmm
	std::string number2(n2); //hmmmm
	std::string sub = (number1.length()>number2.length())? number1 : number2;
	int differenceInLength = abs( (int)(number1.size() - number2.size()) );

	if(number1.size() > number2.size())	
		number2.insert(0, differenceInLength, '0');

	else
		number1.insert(0, differenceInLength, '0');

	for(int i=number1.length()-1; i>=0; --i)
	{
		if(number1[i] < number2[i])
		{
			number1[i] += 10;
			number1[i-1]--;
		}
		sub[i] = ((number1[i]-'0')-(number2[i]-'0')) + '0';
	}

	while(sub[0]=='0' && sub.length()!=1) // erase leading zeros
		sub.erase(0,1);

	return sub;
}

//-------------------------------------------------------------
// multiplies two strings and returns their sum in as a string
std::string BigInteger::multiply(const std::string& num1, const std::string& num2) const
{
	std::string n1(num1); //hmmmm
	std::string n2(num2); //hmmmm
	if(n1.length() > n2.length()) 
		n1.swap(n2);

	std::string res = "0";
	for(int i=n1.length()-1; i>=0; --i)
	{
		std::string temp = n2;
		int currentDigit = n1[i]-'0';
		int carry = 0;

		for(int j=temp.length()-1; j>=0; --j)
		{
			temp[j] = ((temp[j]-'0') * currentDigit) + carry;

			if(temp[j] > 9)
			{
				carry = (temp[j]/10);
				temp[j] -= (carry*10);
			}
			else
				carry = 0;

			temp[j] += '0'; // back to string mood
		}

		if(carry > 0)
			temp.insert(0, 1, (carry+'0'));
		
		temp.append((n1.length()-i-1), '0'); // as like mult by 10, 100, 1000, 10000 and so on

		res = add(res, temp); // O(n)
	}

	while(res[0] == '0' && res.length()!=1) // erase leading zeros
		res.erase(0,1);

	return res;
}

// multiplies strings by integer and returns their sum in as a string
std::string BigInteger::multiply(const std::string& n1, const long long n2) const
{
	std::string res = "0";
	for(int i=n1.length()-1; i>=0; --i)
	{
		std::string temp = std::to_string(n2);

		temp.append((n1.length()-i-1), '0'); // as like mult by 10, 100, 1000, 10000 and so on

		res = add(res, temp); // O(n)
	}

	while(res[0] == '0' && res.length()!=1) // erase leading zeros
		res.erase(0,1);

	return res;
}

//-------------------------------------------------------------
// divides string on long long, returns pair(qutiont, remainder)
std::pair<std::string, long long> BigInteger::divide(const std::string& n, long long den)
{
	long long rem = 0;
	std::string result; //result.resize(MAX);
	
	for(int indx=0, len = n.length(); indx<len; ++indx)
	{
		rem = (rem * 10) + (n[indx] - '0');
		result[indx] = rem / den + '0';
		rem %= den;
	}
	result.resize( n.length() );

	while( result[0] == '0' && result.length() != 1)
		result.erase(0,1);

	if(result.length() == 0)
		result = "0";

	return make_pair(result, rem);
}

//-------------------------------------------------------------
// converts long long to string
std::string BigInteger::toString(long long n) const
{
	std::stringstream ss;
	std::string temp;

	ss << n;
	ss >> temp;

	return temp;
}

//-------------------------------------------------------------
// converts string to long long
long long BigInteger::toInt(const std::string& s) const
{
	long long sum = 0;

	for(size_t i=0; i<s.length(); i++)
		sum = (sum*10) + (s[i] - '0');

	return sum;
}

#endif
