#ifndef GSLERRORS_H
#define GSLERRORS_H

class GslErrors
{

public:

	static const char *name(int errNr)
	{
		switch (errNr)
		{
			case ( 0) : return "Success";
			case (-1) : return "Failure";
			case (-2) : return "Continue";
			case ( 1) : return "Edom";
			case ( 2) : return "Erange";
			case ( 3) : return "Efault";
			case ( 4) : return "Einval";
			case ( 5) : return "Efailed";
			case ( 6) : return "Efactor";
			case ( 7) : return "Esanity";
			case ( 8) : return "Enomem";
			case ( 9) : return "Ebadfunc";
			case (10) : return "Erunaway";
			case (11) : return "Emaxiter";
			case (12) : return "Ezerodiv";
			case (13) : return "Ebadtol";
			case (14) : return "Etol";
			case (15) : return "Eundrflw";
			case (16) : return "Eovrflw";
			case (17) : return "Eloss";
			case (18) : return "Eround";
			case (19) : return "Ebadlen";
			case (20) : return "Enotsqr";
			case (21) : return "Esing";
			case (22) : return "Ediverge";
			case (23) : return "Eunsup";
			case (24) : return "Eunimpl";
			case (25) : return "Ecache";
			case (26) : return "Etable";
			case (27) : return "Enoprog";
			case (28) : return "Enoprogj";
			case (29) : return "Etolf";
			case (30) : return "Etolx";
			case (31) : return "Etolg";
			case (32) : return "Eof";
			default   : return "UNKNOWN";
		}
	}
	static const char *descr(int errNr)
	{
		switch (errNr)
		{
			case ( 0) : return "Success";
			case (-1) : return "generic failure";
			case (-2) : return "iteration has not converged";
			case ( 1) : return "input domain error e.g sqrt(-1)";
			case ( 2) : return "output range error e.g. exp(1e100)"; 
			case ( 3) : return "invalid pointer";
			case ( 4) : return "invalid argument supplied by user";
			case ( 5) : return "generic failure";
			case ( 6) : return "factorization failed";
			case ( 7) : return "sanity check failed - shouldn't happen";
			case ( 8) : return "malloc failed";
			case ( 9) : return "problem with user-supplied function";
			case (10) : return "iterative process is out of control";
			case (11) : return "exceeded max number of iterations";
			case (12) : return "tried to divide by zero";
			case (13) : return "user specified an invalid tolerance";
			case (14) : return "failed to reach the specified tolerance";
			case (15) : return "underflow";
			case (16) : return "overflow";
			case (17) : return "loss of accuracy";
			case (18) : return "failed because of roundoff error";
			case (19) : return "matrix vector lengths are not conformant";
			case (20) : return "matrix not square";
			case (21) : return "apparent singularity detected";
			case (22) : return "integral or series is divergent";
			case (23) : return "requested feature is not supported by the hardware";
			case (24) : return "requested feature not (yet) implemented";
			case (25) : return "cache limit exceeded";
			case (26) : return "table limit exceeded";
			case (27) : return "iteration is not making progress towards solution";
			case (28) : return "jacobian evaluations are not improving the solution";
			case (29) : return "cannot reach the specified tolerance in f";
			case (30) : return "cannot reach the specified tolerance in x";
			case (31) : return "cannot reach the specified tolerance in gradient";
			case (32) : return "end of file";
			default   : return "UNKNOWN";
		}
	}
};
#endif
