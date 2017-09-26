#include "lnstruct.hpp"

#include <queue>
#include <set>
#include <stack>

////////////////////////////////////////////////////////////////////////////////
// lnstruct
//

//non-recursively deletes deeply nested structure of lnstructs
dvl::lnstruct::~lnstruct()
{
	//check if free is valid for deallocation of objects
	std::queue<lnstruct*> q;
	q.push(next);
	q.push(child);

	while(!q.empty())
	{
		lnstruct *ln = q.front();
		q.pop();

		if(ln == nullptr)
			continue;

		q.push(ln->next);
		q.push(ln->child);

		ln->next = nullptr;
		ln->child = nullptr;

		delete ln;
	}
}

int
dvl::lnstruct::level_count()
	const
{
	lnstruct *ln = const_cast<lnstruct*>(this);
	int ct = 0;

	while(ln != nullptr)
	{
		ct++;
		ln = ln->get_next();
	}

	return ct;
}

int
dvl::lnstruct::height()
	const
{
	lnstruct *ln = const_cast<lnstruct*>(this);
	int ct = 0;

	while(ln != nullptr)
	{
		ct++;
		ln = ln->get_child();
	}

	return ct;
}

int
dvl::lnstruct::total_count()
	const
{
	std::stack<lnstruct*> st;
	st.push(const_cast<lnstruct*>(this));

	int ct = 0;

	while(!st.empty())
	{
		if(st.top()->get_child() != nullptr)
			st.push(st.top()->get_child());
		else if(st.top()->get_next() != nullptr)
		{
			lnstruct *n = st.top()->get_next();
			st.pop();
			st.push(n);

			ct++;
		}
		else
		{
			while(!st.empty() && st.top()->get_next() == nullptr)
			{
				st.pop();
				ct++;
			}

			if(!st.empty())
			{
				lnstruct *tmp = st.top();
				st.pop();
				st.push(tmp->get_next());

				ct++;
			}
		}
	}

	return ct;
}

bool
dvl::lnstruct::is_tree()
	const
{
	std::set<lnstruct*> s;
	std::stack<lnstruct*> st;
	st.push(const_cast<lnstruct*>(this));

	while(!st.empty())
	{
		lnstruct *ln = st.top();

		if(s.find(ln) != s.end())
			return false;

		s.emplace(ln);

		if(ln->get_child() != nullptr)
			st.push(ln->get_child());

		if(ln->get_next() != nullptr)
			st.push(ln->get_next());
	}

	return true;
}
