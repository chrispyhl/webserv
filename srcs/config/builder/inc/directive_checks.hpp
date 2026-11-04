#pragma once

#include "Directive.hpp"

// generic checks and nginx-worded errors shared by all directive handlers
namespace config
{
	void bad_directive_throw(const Directive &d);
	void duplicate_throw(const Directive &d);
	void invalid_args_size_throw(const Directive &d);
	void check_for_illegal_block(const Directive &d);
}
