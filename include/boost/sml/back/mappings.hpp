//
// Copyright (c) 016 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_SML_BACK_MAPPINGS_HPP
#define BOOST_SML_BACK_MAPPINGS_HPP

#include "boost/sml/back/transitions.hpp"

namespace back {

template <class>
class sm;

template <class>
struct state;

template <class>
struct event;

template <class...>
struct transitions;

template <class...>
struct transitions_sub;

template <class, class>
struct state_mappings;

template <class S, class... Ts>
struct state_mappings<S, aux::type_list<Ts...>> {
  using element_type = state<S>;
  using types = aux::type_list<Ts...>;
};

template <class, class>
struct event_mappings;

template <class E, class... Ts>
struct event_mappings<E, aux::type_list<Ts...>> {
  using element_type = event<E>;
  using types = aux::type_list<Ts...>;
};

template <class...>
struct unique_mappings;

template <class, class...>
struct unique_mappings_impl;

template <class... Ts>
using unique_mappings_t = typename unique_mappings<Ts...>::type;

template <class, class, class, class R>
struct get_mapping : aux::type_list<R> {};

template <class E, class T, class R>
struct get_mapping<event<E>, event<E>, T, R>
    : aux::type_list<event_mappings<E, aux::apply_t<unique_mappings_t, aux::join_t<typename R::types, typename T::types>>>> {};

template <class S, class T, class R>
struct get_mapping<state<S>, state<S>, T, R>
    : aux::type_list<state_mappings<S, aux::join_t<typename R::types, typename T::types>>> {};

template <class T, class... Ts>
struct extend_mapping : aux::join_t<typename get_mapping<typename T::element_type, typename Ts::element_type, T, Ts>::type...> {
};

template <class T, class... Ts>
using extend_mapping_t = aux::apply_t<aux::inherit, typename extend_mapping<T, Ts...>::type>;

template <bool, class, class...>
struct conditional_mapping;

template <class T1, class T, class... Rs, class... Ts>
struct conditional_mapping<true, aux::type<T1, aux::inherit<Rs...>>, T, Ts...> {
  using type = unique_mappings_impl<aux::type<aux::inherit<T1>, extend_mapping_t<T, Rs...>>, Ts...>;
};

template <class T1, class T, class... Rs, class... Ts>
struct conditional_mapping<false, aux::type<T1, aux::inherit<Rs...>>, T, Ts...> {
  using type =
      unique_mappings_impl<aux::type<aux::inherit<T1, aux::type<typename T::element_type>>, aux::inherit<T, Rs...>>, Ts...>;
};

template <class T1, class T, class... Rs, class... Ts>
struct unique_mappings_impl<aux::type<T1, aux::inherit<Rs...>>, T, Ts...>
    : conditional_mapping<aux::is_base_of<aux::type<typename T::element_type>, T1>::value, aux::type<T1, aux::inherit<Rs...>>,
                          T, Ts...>::type {};

template <class T1, class Rs>
struct unique_mappings_impl<aux::type<T1, Rs>> : aux::apply_t<aux::inherit, Rs> {};

template <class... Ts>
struct unique_mappings : unique_mappings_impl<aux::type<aux::none_type, aux::inherit<>>, Ts...> {};

template <class T>
struct unique_mappings<T> : aux::inherit<T> {};

template <class, class TDefault>
TDefault get_event_mapping_impl(...);

template <class T, class, class TMappings>
TMappings get_event_mapping_impl(event_mappings<T, TMappings> *);

template <class T, class TDefault, class TMappings>
using get_event_mapping_t = decltype(get_event_mapping_impl<T, TDefault>((TMappings *)0));

template <class, class, class>
struct mappings;

template <class, class, class>
struct remap;

template <class TUnexpected, class, class, class>
struct get_transition {
  using type = back::transitions<TUnexpected>;
};

template <class TUnexpected, class TState, class... Ts>
struct get_transition<TUnexpected, TState, TState, aux::type_list<Ts...>> {
  using type = back::transitions<Ts...>;
};

template <class TUnexpected, class TState, class... Ts>
struct get_transition<TUnexpected, sm<TState>, sm<TState>, aux::type_list<Ts...>> {
  using type = back::transitions_sub<sm<TState>, Ts...>;
};

template <class TUnexpected, class... TStates, class TState, class Ts>
struct remap<TUnexpected, aux::type_list<TStates...>, back::state_mappings<TState, Ts>> {
  using type = aux::type_list<typename get_transition<TUnexpected, TStates, TState, Ts>::type...>;
};

template <class TUnexpected, class... Ts, class TStates>
struct mappings<TUnexpected, aux::pool<Ts...>, TStates>
    : back::unique_mappings_t<back::event_mappings<
          typename Ts::event, typename remap<TUnexpected, TStates, typename back::state_mappings<
                                                                       typename Ts::src_state, aux::type_list<Ts>>>::type>...> {
};

template <class TUnexpected, class Ts, class TStates>
using mappings_t = typename mappings<TUnexpected, Ts, TStates>::type;

template <class, class>
struct default_mappings;

template <class TUnexpected, class... TStates>
struct default_mappings<TUnexpected, aux::type_list<TStates...>>
    : aux::type_list<aux::expand_t<TStates, transitions<TUnexpected>>...> {};

template <class TUnexpected, class TStates>
using default_mappings_t = typename default_mappings<TUnexpected, TStates>::type;

}  // back

#endif
