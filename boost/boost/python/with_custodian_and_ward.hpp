// Copyright David Abrahams 2002. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef WITH_CUSTODIAN_AND_WARD_DWA2002131_HPP
# define WITH_CUSTODIAN_AND_WARD_DWA2002131_HPP

# include <boost/python/detail/prefix.hpp>

# include <boost/python/default_call_policies.hpp>
# include <boost/python/object/life_support.hpp>
# include <algorithm>

namespace boost { namespace python { 

namespace detail
{
  template <std::size_t N>
  struct get_prev
  {
      template <class ArgumentPackage>
      static PyObject* execute(ArgumentPackage const& args, PyObject* = 0)
      {
          int const pre_n = static_cast<int>(N) - 1; // separate line is gcc-2.96 workaround
          return detail::get(mpl::int_<pre_n>(), args);
      }
  };
  template <>
  struct get_prev<0>
  {
      template <class ArgumentPackage>
      static PyObject* execute(ArgumentPackage const&, PyObject* zeroth)
      {
          return zeroth;
      }
  };
}
template <
    std::size_t custodian
  , std::size_t ward
  , class BasePolicy_ = default_call_policies
>
struct with_custodian_and_ward : BasePolicy_
{
    BOOST_STATIC_ASSERT(custodian != ward);
    BOOST_STATIC_ASSERT(custodian > 0);
    BOOST_STATIC_ASSERT(ward > 0);

    template <class ArgumentPackage>
    static bool precall(ArgumentPackage const& args_)
    {
        unsigned arity_ = detail::arity(args_);
        if (custodian > arity_ || ward > arity_)
        {
            PyErr_SetString(
                PyExc_IndexError
              , "boost::python::with_custodian_and_ward: argument index out of range"
            );
            return false;
        }

        PyObject* patient = detail::get_prev<ward>::execute(args_);
        PyObject* nurse = detail::get_prev<custodian>::execute(args_);

        PyObject* life_support = python::objects::make_nurse_and_patient(nurse, patient);
        if (life_support == 0)
            return false;
    
        bool result = BasePolicy_::precall(args_);

        if (!result)
            Py_DECREF(life_support);
    
        return result;
    }
};

template <std::size_t custodian, std::size_t ward, class BasePolicy_ = default_call_policies>
struct with_custodian_and_ward_postcall : BasePolicy_
{
    BOOST_STATIC_ASSERT(custodian != ward);
    
    template <class ArgumentPackage>
    static PyObject* postcall(ArgumentPackage const& args_, PyObject* result)
    {
        std::size_t arity_ = detail::arity(args_);
        if ( custodian > arity_ || ward > arity_ )
        {
            PyErr_SetString(
                PyExc_IndexError
              , "boost::python::with_custodian_and_ward_postcall: argument index out of range"
            );
            return 0;
        }
        
        PyObject* patient = detail::get_prev<ward>::execute(args_, result);
        PyObject* nurse = detail::get_prev<custodian>::execute(args_, result);

        if (nurse == 0) return 0;
    
        result = BasePolicy_::postcall(args_, result);
        if (result == 0)
            return 0;
            
        if (python::objects::make_nurse_and_patient(nurse, patient) == 0)
        {
            Py_XDECREF(result);
            return 0;
        }
        return result;
    }
};


}} // namespace boost::python

#endif // WITH_CUSTODIAN_AND_WARD_DWA2002131_HPP
