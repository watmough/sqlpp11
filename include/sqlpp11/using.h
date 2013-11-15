/*
 * Copyright (c) 2013, Roland Bock
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLPP_USING_H
#define SQLPP_USING_H

#include <ostream>
#include <sqlpp11/type_traits.h>
#include <sqlpp11/detail/set.h>
#include <sqlpp11/detail/serialize_tuple.h>
#include <sqlpp11/detail/serializable.h>

namespace sqlpp
{
	template<typename... Table>
		struct using_t
		{
			using _is_using = std::true_type;

			static_assert(sizeof...(Table), "at least one table argument required in using()");

			// check for duplicate arguments
			static_assert(not detail::has_duplicates<Table...>::value, "at least one duplicate argument detected in using()");

			// check for invalid arguments
			using _valid_expressions = typename detail::make_set_if<is_table_t, Table...>::type;
			static_assert(_valid_expressions::size::value == sizeof...(Table), "at least one argument is not an table in using()");


			template<typename Db>
				void serialize(std::ostream& os, Db& db) const
				{
					os << " USING ";
					detail::serialize_tuple(os, db, _tables, ',');
				}

			std::tuple<Table...> _tables;
		};

	template<typename Db>
	struct dynamic_using_t
	{
		using _is_using = std::true_type;
		using _is_dynamic = std::true_type;

		template<typename Table>
		void add(Table&& table)
		{
			static_assert(is_table_t<typename std::decay<Table>::type>::value, "using arguments require to be tables");
			_dynamic_tables.push_back(std::forward<Table>(table));
		}

		void serialize(std::ostream& os, Db& db) const
		{
			if (_dynamic_tables.empty())
				return;
			os << " USING ";
			bool first = true;
			for (const auto& table : _dynamic_tables)
			{
				if (not first)
					os << ',';
				table.serialize(os, db);
				first = false;
			}
		}

		std::vector<detail::serializable_t<Db>> _dynamic_tables;
	};
}

#endif