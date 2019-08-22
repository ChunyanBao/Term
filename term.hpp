#ifndef TERM_HPP
#define TERM_HPP

#include <iostream>
#include <vector>
#include <iterator>
#include <stack>
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>

using namespace std;

//forward declare the classes
template<typename T>
class term;
template<typename T>
class literal;
template<typename T>
class variable;
template<typename T>
class function;
template<typename T>
class term_iterator;
template<typename T>
class Sub;

template<typename T>
using term_ptr = std::shared_ptr<term<T>>;
template<typename T>
using literal_ptr = std::shared_ptr<literal<T>>;
template<typename T>
using variable_ptr = std::shared_ptr<variable<T>>;
template<typename T>
using function_ptr = std::shared_ptr<function<T>>;

// a rule is a pair of terms
template<typename T>
using rule = pair<term_ptr<T>, term_ptr<T>>;


template<typename T>
class term
{
	public:
		typedef term<T>  value_type;
		typedef term<T>* pointer;
		typedef term<T>& reference;
		typedef size_t   size_type;
		typedef ptrdiff_t difference_type;
		typedef std::bidirectional_iterator_tag   iterator_category;
		typedef term_iterator<T>  iterator;
		typedef term_iterator<const T>  const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		
		term(){}; 

		//iterator	
		iterator begin() {return term_iterator<T>(this);}
		iterator end() {return term_iterator<T>();}
		const_iterator cbegin() {return term_iterator<T>(this);}
		const_iterator cend() {return term_iterator<T>();}
		reverse_iterator rbegin() {return std::reverse_iterator<iterator>(end());}
		reverse_iterator rend() {return std::reverse_iterator<iterator>(begin());}

		const_reverse_iterator crbegin()
		{
			return std::reverse_iterator<const_iterator>(cend());
		}

		const_reverse_iterator crend()
		{
			return std::reverse_iterator<const_iterator>(cbegin());
		}
		
		//function to check the term is literal or variable or function	
		virtual bool is_literal()= 0;
		virtual bool is_variable() = 0;
		virtual bool is_function() = 0;

		//get name
		virtual string get_name() = 0;
		//get subterms
		virtual vector<shared_ptr<term<T>>> &get_subterms() = 0;
		virtual std::ostream & print(std::ostream &)const = 0;
		
		// a functiont to convert term * to shared ptr	
		virtual shared_ptr<term<T>> convert() = 0;

		//rewrite functions
		shared_ptr<term<T>> rewrite(shared_ptr<term<T>> t, vector<int>  p, Sub<T> & sigma);
		shared_ptr<term<T>> rewrite(shared_ptr<term<T>> c, shared_ptr<term<T>> t, vector<int> p, Sub<T> & sigma);

		//a function to replace the shared ptr term t according to sigma
		shared_ptr<term<T>> replace(Sub<T> & sigma);
		
		//three functions which are called in the unify function 
		//if t1 is literal check t2 is: literal? or variable? or function
		bool checkLiteral(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
		//if t1 is variable check t2 is: literal? or variable? or function
		bool checkVariable(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
		//if t1 is function check t2 is: literal? or variable? or function
		bool checkFunction(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);

		bool unify(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
};

template<typename T>
bool checkLiteral(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
template<typename T>
bool checkVariable(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
template<typename T>
bool checkFunction(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);
template<typename T>
bool unify(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma);

//every variable in t need to be replaced
//it is garuanteed that all the variable are in sigma

template<typename T>
shared_ptr<term<T>> term<T>::replace(Sub<T> & sigma)
{
	vector<shared_ptr<term<T>>> children;
	if(this->is_literal())
	{
	//	literal<T> copy(static_cast<literal<T>&>*this);
		literal<T> copy((literal<T>&)*this);

		return make_shared<literal<T>>(literal<T>(copy));
	}
	else if(this->is_variable()){
		return  sigma(this->get_name());	
	}
	else
	{
		for(size_t i = 0; i < (*this).get_subterms().size(); ++i)
		{
			children.push_back((*this).get_subterms()[i]->replace(sigma));
		}
	
		return make_shared<function<T>>(function<T>(this->get_name(), children.size(), children));
	}
}

template<typename T>
shared_ptr<term<T>> term<T>::rewrite( shared_ptr<term<T>>  t, vector<int> p, Sub<T> & sigma)
{
	shared_ptr<term<T>> c;
	c = this->convert();
	rewrite(c, t, p, sigma);
	return c;
}

template<typename T>
shared_ptr<term<T>> term<T>::rewrite(shared_ptr<term<T>> c, shared_ptr<term<T>> t, vector<int> p, Sub<T> & sigma)
{
	if((c->is_variable() || c->is_literal()) && !p.empty())
	{
		throw " path is not empty but we are at a varialbe or liberal";
	}
	if(p.empty())
	{
		c = t->replace(sigma);
		return c;
	}
	int val = --p[0];
	p.erase(p.begin());
	c->get_subterms()[val] = c->get_subterms()[val]->rewrite(c->get_subterms()[val], t, p, sigma);

	return c;
}

template<typename T>
bool checkLiteral(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma)
{
	if(t2->is_literal())
	{
		literal<T> c_t1((literal<T>&)*t1);
		literal<T> c_t2((literal<T>&)*t2);
		if(c_t1.get_val() != c_t2.get_val())
			return false;
		else
			return true;
	}
	else if(t2->is_variable())
	{
		sigma.extend(t2->get_name(), t1);
		return true;
	}
	//if(t2->is_function())
	else
	{
		return false;
	}
}

template<typename T>
bool checkVariable(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma)
{
	sigma.extend(t1->get_name(), t2);
	
	return true;
}
template<typename T>
bool checkFunction(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma)
{
	if(t2->is_literal())
		return false;
	else if(t2->is_variable())
	{
		sigma.extend(t2->get_name(), t1);
		return true;
	}
	else
	{
		if(t1->get_name() != t2->get_name())
			return false;

		if(t1->get_subterms().size() != t2->get_subterms().size())
			return false;

		for(size_t i = 0; i < t1->get_subterms().size(); ++i)
		{
			if(unify(t1->get_subterms()[i], t2->get_subterms()[i], sigma) == false)
				return false;
		}
		return true;
	}

}

template<typename T>
bool unify(shared_ptr<term<T>> t1, shared_ptr<term<T>> t2, Sub<T> & sigma)
{
	if(t1->is_literal())
	{
		return checkLiteral(t1, t2, sigma);
	}

	else if(t1->is_variable())	
	{
		return checkVariable(t1, t2, sigma);
	}
	else
        {
		return checkFunction(t1, t2, sigma);
	}
}

template<typename T>
class literal: public term<T>
{
	private:
		T value;
		vector<shared_ptr<term<T>>> v;

	public:
		literal();
		literal(T c): value(c){}
		literal(const literal & l): value(l.value){}

		literal & operator = (const literal & l);
		T get_val(){return value;};
		
		bool is_literal(){return true;}
		bool is_variable(){return false;}
		bool is_function(){return false;}
		string get_name() {return "";}

		shared_ptr<term<T>> convert();

		vector<shared_ptr<term<T>>> & get_subterms() {return v;}
		ostream & print(ostream & out)const;
};

template<typename T>
literal<T> & literal<T>::operator = (const literal & l)
{
	if(this == & l) return *this;
	value = l.value;
}

template<typename T>
ostream & literal<T>::print(ostream & out)const
{
	out << value;
	out.flush();
	return out;
}

template<typename T>
shared_ptr<term<T>> literal<T>::convert()
{
	return make_shared<literal<T>> (*this);
}

template<typename T>
class variable: public term<T>
{
	private:
		std::string name;
		vector<shared_ptr<term<T>>> v;
	public:
		variable();
		variable(std::string c):name(c){}
		variable(const variable & a):name(a.name){}

		variable & operator = (const variable & v);

		std::string get_name(){return name;}

		bool is_literal(){return false;}
		bool is_variable(){return true;}
		bool is_function(){return false;}
		vector<shared_ptr<term<T>>> &get_subterms() {return v;}

		shared_ptr<term<T>> convert();

		std::ostream & print(std::ostream & out)const;
};

template<typename T>
variable<T> & variable<T>::operator = (const variable & v)
{
	if(this == & v) return *this;
	name = v.name;
}

template<typename T>
std::ostream & variable<T>:: print(std::ostream & out)const
{
	out << name;
	out.flush();
	return out;
}

template<typename T>
shared_ptr<term<T>> variable<T>::convert()
{
	return make_shared<variable<T>>(*this);
}


template<typename T>
std::ostream & operator<<(std::ostream & out, const term<T> & v)
{
	v.print(out);
	return out;
}

template<typename T>
class function: public term<T>
{
	private:
		string _name;
		int _num_arg;
		vector<shared_ptr<term<T>>> _subterms;

	public:
		function():_name(""), _num_arg(0), _subterms(){};
		function(string name, int num, vector<shared_ptr<term<T>>> arguments);
		function(const function & a);

		function & operator = (const function & f);

		string get_name(){return _name;}
		int get_num(){return _num_arg;}
		vector<shared_ptr<term<T>>> &get_subterms(){return _subterms;};
		shared_ptr<term<T>> convert();

		bool is_literal(){return false;}
		bool is_variable(){return false;}
		bool is_function(){return true;}
		
		ostream & print(ostream &)const;
};

template<typename T>
function<T>::function(string name, int num, vector<shared_ptr<term<T>>> arguments)
{
	_name = name;
	_num_arg = num;
	_subterms.resize(_num_arg);
	for(int i = 0; i < num; ++i)
	{
		_subterms[i] = arguments[i];
	}
}

template<typename T>
function<T> & function<T>::operator = (const function & f)
{
	if(this == &f) return *this;
	_name = f._name;
	_num_arg = f._num_arg;
	for(int i = 0; i < _num_arg; ++i)
	{
		_subterms[i] = f._subterms[i];
	}
}

template<typename T>
shared_ptr<term<T>> function<T>::convert()
{
	return make_shared<function<T>>(*this);
}

template<typename T>
function<T>::function(const function & a)
{
	_name = a._name;
	_num_arg = a._num_arg;
	_subterms = a._subterms;
}

template<typename T>
ostream & function<T>::print(ostream & out)const
{
	out << _name << "("; out.flush();
	if(!_subterms.empty())	
	{
		for(size_t i = 0; i < _subterms.size()-1; ++i)
		{
			out <<*_subterms[i] << ", "; out.flush();
		}
		out << *_subterms[_subterms.size()-1]; out.flush();
	}
	out << ")"; out.flush();
	return out;
}

template<typename T>
class term_iterator
{
	private:
		stack<term<T> *> _path;	
	public:
		term_iterator(){}
		term_iterator(term<T> * root);

		term_iterator(const term_iterator<T> & i): _path(i._path){}  

		
		term<T> & operator*() const {return *_path.top();};
		term<T> * operator->() const {return _path.top();};
		term_iterator & operator++();
		term_iterator & operator--();
		term_iterator operator++(int);
		term_iterator & operator+=(unsigned int);
		term_iterator & operator-=(unsigned int);
		bool operator == (const term_iterator<T> & rhs)const
		{
			return _path == rhs._path;
		}
		bool operator != (const term_iterator<T> & rhs)const
		{
			return _path != rhs._path;
		}
};

template<typename T>
term_iterator<T>::term_iterator(term<T> * root)

{
	if(root)
	{
		_path.push(root);
	}
}

template<typename T>
term_iterator<T> & term_iterator<T>::operator++()
{
	if(!_path.empty())
	{
		if(_path.top()->is_function())
		{
			term<T> * node = _path.top();
			_path.pop();

			
			for(int i = node->get_subterms().size()-1; i >= 0; --i)
			{
				_path.push(node->get_subterms()[i].get());
			}
		}
		else
		{
			_path.pop();
		}
	}
	return *this;
}

template<typename T>
term_iterator<T> term_iterator<T>:: operator++(int)
{
	term_iterator<T> temp(*this);
		
	++(*this);
	return temp;
}

template<typename T>
term_iterator<T> & term_iterator<T>::operator+=(unsigned int num)
{
	for(unsigned int i = 0; i < num; ++i)
	{
		++(*this);
	}
	return *this;
}

/////////////////////////////////
// Boolean algebra
/////////////////////////////////

//variables in terms
term_ptr<bool> b_v() {return make_shared<variable<bool>>(variable<bool>("v"));}
term_ptr<bool> b_w() {return make_shared<variable<bool>>(variable<bool>("w"));}
term_ptr<bool> b_x() {return make_shared<variable<bool>>(variable<bool>("x"));}
term_ptr<bool> b_y() {return make_shared<variable<bool>>(variable<bool>("y"));}
term_ptr<bool> b_z() {return make_shared<variable<bool>>(variable<bool>("z"));}

//literal values
term_ptr<bool> b_true() {return make_shared<literal<bool>>(literal<bool>(true));}
term_ptr<bool> b_false() {return make_shared<literal<bool>>(literal<bool>(false));}

//functions
term_ptr<bool> b_and(term_ptr<bool> x, term_ptr<bool> y)
{
    return make_shared<function<bool>>(function<bool>("&&",2, {x, y}));
}
term_ptr<bool> b_or(term_ptr<bool> x, term_ptr<bool> y)
{
    return make_shared<function<bool>>(function<bool>("||", 2, {x, y}));
}
term_ptr<bool> b_not(term_ptr<bool> x)
{
    return make_shared<function<bool>>(function<bool>("!", 1, {x}));
}
term_ptr<bool> b_arrow(term_ptr<bool> x, term_ptr<bool> y)
{
    return make_shared<function<bool>>(function<bool>("->", 2, {x,y}));
}

//variables for rules (to make sure there's no overlap)
//variables for rewrite rules are a and b
variable_ptr<bool> b_a() {return make_shared<variable<bool>>(variable<bool>("a"));}
variable_ptr<bool> b_b() {return make_shared<variable<bool>>(variable<bool>("b"));}

// the rule ->(a, false) => !(a)
rule<bool> contra(b_arrow(b_a(), b_false()), b_not(b_a()));

// the actual terms we'll be using
term_ptr<bool> b1 = b_or(b_and(b_x(), b_false()), b_y());
term_ptr<bool> b2 = b_or(b_and(b_true(), b_x()), b_arrow(b_or(b_v(), b_w()), b_false()));


/////////////////////////////////
// substitution
/////////////////////////////////

/**
 * a simple implementation of a substitution class.
 * contains an extend method and an operator()
 *
 * if I have:
 * Sub s;
 * s.extend("x", t);
 * cout << s("x") << endl;
 * I'll print out t.
 */


template<typename T>
class Sub
{
    unordered_map<string, term_ptr<T>> _map;

public:
    term_ptr<T> operator()(string s)
    {
        return _map.at(s);
    }
    void extend(string s, term_ptr<T> t)
    {
        _map[s] = t;
    }

    // print the substitution so I can verify that unify works
    void print()
    {
        cout << "[" << endl;
        for(pair<string, term_ptr<T>> p : _map)
        {
            cout << p.first << " :-> " <<(* p.second )<< endl;
        }
        cout << "]" << endl;
    }
    bool contains(string s)
    {
	    return _map.count(s);
    }
};




#endif
