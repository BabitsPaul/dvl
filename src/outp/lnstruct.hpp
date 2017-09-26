#ifndef OUTP_LNSTRUCT_HPP_
#define OUTP_LNSTRUCT_HPP_

#include "../ex.hpp"

namespace dvl
{
	////////////////////////////////////////////////////////////////////////////
	// lnstruct
	//

	/**
	 * Represents a structure that was found while parsing an input-stream. The structure
	 * is represented by offset in the stream and offset of the end of the structure in
	 * the stream. It also keeps pointers to the next and child-struct of this lnstruct, thus
	 * building a nested tree-like structure made of linked lists.
	 *
	 * Every lnstruct is associated with a type of routine by the corresponding @link pid.
	 *
	 * @see routine
	 * @see pid
	 */
	class lnstruct
	{
	private:
		/**
		 * Starting-offset of the structure in the input-stream
		 */
		long start;

		/**
		 * End-offset of the structure in the input-stream (inclusive)
		 */
		long end;

		/**
		 * PID of the routine that parsed this structure
		 */
		pid id;

		/**
		 * Next and child-lnstructs of this structure.
		 */
		lnstruct *next, *child;
	public:
		/**
		 * Constructs a new lnstruct with the specified pid and start-offset.
		 * End and associated lnstructs will be set to default-values (0/nullptr)
		 */
		lnstruct(pid id, long start):start(start), end(0l), id(id),
										next(nullptr), child(nullptr){}
		virtual ~lnstruct();

		/**
		 * Updates the end of the lnstruct to the specified value. Note that this value must
		 * be greater/equal than the start, as otherwise the lnstruct will be invalid and a
		 * parser_exception will be thrown.
		 *
		 * @see parser_exception
		 */
		void set_end(long end)
			throw(parser_exception)
		{
			if(end < start)
				throw parser_exception(id, "Invalid end specification - must be >= start");

			this->end = end;
		}

		/**
		 * Accessor for the starting-index of this lnstruct.
		 *
		 * @see start
		 */
		long get_start() const {return start;}

		/**
		 * Accessor for the end-index of this lnstruct
		 *
		 * @see end
		 */
		long get_end() const {return end;}


		/**
		 * Accessor for the lnstruct following this structure
		 *
		 * @see next
		 */
		lnstruct*& get_next(){ return next; }

		/**
		 * Accessor for the child-lnstruct of this structure
		 *
		 * @see child
		 */
		lnstruct*& get_child(){ return child; }

		/**
		 * Generates a wstring representing the structure associated with this
		 * string in an indentation-based tree-view.
		 *
		 * @param indent the indent used for this structure (child-structure will receive indent + "\t")
		 */
		std::wstring structure(pid_table &pt, std::wstring indent=L"") const
		{
			std::wstring result = L"";
			result.append(indent);
			result.append(pt.to_string(id, true));
			result.append(L" start=").append(std::to_wstring(start));
			result.append(L" end=").append(std::to_wstring(end));
			result.append(L"\n");

			if(child != nullptr)
				result.append(child->structure(pt, indent + L"\t"));

			if(next != nullptr)
				result.append(next->structure(pt, indent));

			return result;
		}

		/**
		 * Counts the total number of lnstructs that are on the same level with
		 * this lnstruct and follow it. Counts the number of next elements that are
		 * not null.
		 *
		 * @return the number of elements on the same level as this node
		 */
		int level_count() const;

		/**
		 * Calculates the total number of lnstructs within the tree of lnstructs
		 * that has this node as root.
		 *
		 * @return number of nodes in the subtree that has this routine as root
		 */
		int total_count() const;

		/**
		 * Calculates the height of this tree as the number of nodes "vertically above"
		 * this node.
		 *
		 * @return the height of the subtree
		 */
		int height() const;

		bool is_tree() const;
	};
}

#endif /* OUTP_LNSTRUCT_HPP_ */
