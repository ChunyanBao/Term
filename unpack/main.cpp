#include "term.hpp"

using namespace std;


int main()
{
	shared_ptr<term<bool>> t1 = b_v(); 	
	shared_ptr<term<bool>> t3; // check for assignment operator

	shared_ptr<term<bool>> t2 = b_and(b_a(), b_a());

	shared_ptr<term<bool>> t4 = b_and(b_v(), t2);

	shared_ptr<term<bool>> t5, tt;
	tt = t4;
	cout << "checking for assignment operator: " << endl;
	cout << *tt << endl << endl;
	
	cout << "checking for iterator: " << endl;
	cout << "print *t2" << endl;
	cout << *t2 << endl << endl;

	cout << "use iterator to print *t2" << endl;
	for(auto t = t2->begin(); t != t2->end(); t++)
	{
		cout << *t << endl;
	}	
	
	cout << endl;
	
    	// make the substitution [a :-> or(v,w)]
    	Sub<bool> sigma;
    	sigma.extend("a", b_or(b_v(),b_w()));


	cout << "checking for rewrite: " << endl;
	cout << "before rewrite: " << endl;
	cout << *t4 << endl;	
	try{
    	t5 = t4->rewrite(t2, {2, 2}, sigma);
	}catch(const char * msg)
	{
		cerr << msg << endl;
		return 0;
	}

	cout << "after rewrite: " << endl;
    	cout << *t5 << endl << endl; 

    	// can I unify t1 with t2?
	shared_ptr<term<bool>> t6 = b_and(b_a(), b_v());
	shared_ptr<term<bool>> t7 = b_and(b_w(), b_x());
	shared_ptr<term<bool>> t8 = b_and(b_y(), t7);

	cout << "\n\n checking for unify: " << endl;
	cout << "* t6 = " << endl;
	cout << *t6 <<endl;

	cout << "* t8 = " << endl;
	cout << *t8 <<endl;

	cout << "* t7 = " << endl;
	cout << *t7 <<endl;


    	Sub<bool> s;
    	cout << "unifying t6 with t8? " << unify(t6,t8,s) << endl;
    	s.print();

	Sub<bool>ss;
	cout << "unifying t8 with t6? " << unify(t8,t6,ss) << endl;
    	ss.print();

	t6 = b_and(b_a(), b_v());
	Sub<bool>sss;
	cout << "unifying t7 with t6? " << unify(t7,t6,sss) << endl;
    	sss.print();


	shared_ptr<term<bool>> t9 = b_and(b_a(), b_true());

	cout << "* t9 = " << endl;
	cout << *t9 <<endl;

	Sub<bool>ssss;
	cout << "unifying t7 with t9? " << unify(t7,t9,ssss) << endl;
    	ssss.print();

	
	
	return 0;
}
