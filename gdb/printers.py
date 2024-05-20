#
# Custom GDB pretty printers for C++23
#

import gdb
import re

#
# util functions
#

def debug(message, *args):
    print("\n-------------------\ndebug message:\n")
    print(message, *args)
    print("\n-------------------")

#
# These are for printing some of the custom data types used by the game
#

class RogueStackVectorPrinter:
    """Print a stack_vector"""

    def __init__(self, val):
        self.val = val
        self.typename = _prettify_typename(val.type)
        self.elems = self.val["arr"]["__elems_"]
    def to_string(self):
        return f"{self.typename}{self.elems}"
    def display_hint(self):
        return "array"

#
# These are for printing some libcxx data types that for some reason
# LLVM doesn't provide already.
#

class StdArrayPrinter:
    """Print a std::array"""

    def __init__(self, val):
        self.val = val
        self.typename = _prettify_typename(val.type)
        self.elems = self.val["__elems_"]
    def to_string(self):
        return f"{self.typename}{self.elems}"
    def display_hint(self):
        return "array"

class StdOptionalPrinter:
    """Print a std::optional"""

    def __init__(self, val):
        self.val = val
        self.typename = _prettify_typename(val.type)
        self.has_value = self.val["__engaged_"]
        if self.has_value:
            self.opt_value = self.val["__val_"]
    def to_string(self):
        if self.has_value:
            return f"{self.typename}({self.opt_value})"
        else:
            return f"{self.typename}::nullopt"
        
class StdInitializerListPrinter:
    """Print a std::initializer_list"""

    def __init__(self, val):
        self.val = val
        self.typename = _prettify_typename(val.type)
        self.begin = self.val["__begin_"]
        self.size = int(self.val["__size_"])

    def to_string(self):
        children = [
            self.begin[index]
            for index
            in range(0, self.size)
        ]
        return f"{self.typename}{{ {', '.join(str(c) for c in children)} }}"

    def display_hint(self):
        return "array"

#
# Everything below here is taken from LLVM's libcxx printers.py
#

# Some common substitutions on the types to reduce visual clutter (A user who
# wants to see the actual details can always use print/r).
_common_substitutions = [
    (
        "std::basic_string<char, std::char_traits<char>, std::allocator<char> >",
        "std::string",
    ),
    ("std::basic_string_view<char, std::char_traits<char> >", "std::string_view"),
]

def _remove_cxx_namespace(typename):
    """Removed libc++ specific namespace from the type.

    Arguments:
      typename(string): A type, such as std::__u::something.

    Returns:
      A string without the libc++ specific part, such as std::something.
    """

    return re.sub("std::__.*?::", "std::", typename)

def _prettify_typename(gdb_type):
    """Returns a pretty name for the type, or None if no name can be found.

    Arguments:
      gdb_type(gdb.Type): A type object.

    Returns:
      A string, without type_defs, libc++ namespaces, and common substitutions
      applied.
    """

    type_without_typedefs = gdb_type.strip_typedefs()
    typename = (
        type_without_typedefs.name
        or type_without_typedefs.tag
        or str(type_without_typedefs)
    )
    result = _remove_cxx_namespace(typename)
    for find_str, subst_str in _common_substitutions:
        result = re.sub(find_str, subst_str, result)
    return result

def _remove_std_prefix(typename):
    match = re.match("^std::(.+)", typename)
    return match.group(1) if match is not None else typename

def _remove_generics(typename):
    """Remove generics part of the type. Assumes typename is not empty.

    Arguments:
      typename(string): A type such as std::my_collection<element>.

    Returns:
      The prefix up to the generic part, such as std::my_collection.
    """

    match = re.match("^([^<]+)", typename)
    return match.group(1)

class RogueCustomPrettyPrinter(object):
    def __init__(self, name):
        super(RogueCustomPrettyPrinter, self).__init__()
        self.name = name
        self.enabled = True

        self.lookup = {
            # Custom data types used in the game

            "stack_vector": RogueStackVectorPrinter,

            # libcxx types not provided by LLVM

            "array": StdArrayPrinter,
            "optional": StdOptionalPrinter,
            "initializer_list": StdInitializerListPrinter,

            # These are already provided by the LLVM printers
            # (leaving them commented here for easy reference)

            # "basic_string": StdStringPrinter,
            # "string": StdStringPrinter,
            # "string_view": StdStringViewPrinter,
            # "tuple": StdTuplePrinter,
            # "unique_ptr": StdUniquePtrPrinter,
            # "shared_ptr": StdSharedPointerPrinter,
            # "weak_ptr": StdSharedPointerPrinter,
            # "bitset": StdBitsetPrinter,
            # "deque": StdDequePrinter,
            # "list": StdListPrinter,
            # "queue": StdQueueOrStackPrinter,
            # "stack": StdQueueOrStackPrinter,
            # "priority_queue": StdPriorityQueuePrinter,
            # "map": StdMapPrinter,
            # "multimap": StdMapPrinter,
            # "set": StdSetPrinter,
            # "multiset": StdSetPrinter,
            # "vector": StdVectorPrinter,
            # "__map_iterator": MapIteratorPrinter,
            # "__map_const_iterator": MapIteratorPrinter,
            # "__tree_iterator": SetIteratorPrinter,
            # "__tree_const_iterator": SetIteratorPrinter,
            # "fpos": StdFposPrinter,
            # "unordered_set": StdUnorderedSetPrinter,
            # "unordered_multiset": StdUnorderedSetPrinter,
            # "unordered_map": StdUnorderedMapPrinter,
            # "unordered_multimap": StdUnorderedMapPrinter,
            # "__hash_map_iterator": StdUnorderedMapIteratorPrinter,
            # "__hash_map_const_iterator": StdUnorderedMapIteratorPrinter,
            # "__hash_iterator": StdUnorderedSetIteratorPrinter,
            # "__hash_const_iterator": StdUnorderedSetIteratorPrinter,
        }

        self.subprinters = []
        for name, subprinter in self.lookup.items():
            # Subprinters and names are used only for the rarely used command "info
            # pretty" (and related), so the name of the first data structure it prints
            # is a reasonable choice.
            if subprinter not in self.subprinters:
                subprinter.name = name
                self.subprinters.append(subprinter)

    def __call__(self, val):
        """Return the pretty printer for a val, if the type is supported."""

        # Do not handle any type that is not a struct/class.
        if val.type.strip_typedefs().code != gdb.TYPE_CODE_STRUCT:
            return None

        # Don't attempt types known to be inside libstdcxx.
        typename = val.type.name or val.type.tag or str(val.type)
        match = re.match("^std::(__.*?)::", typename)
        if match is not None and match.group(1) in [
            "__cxx1998",
            "__debug",
            "__7",
            "__g",
        ]:
            return None

        # Handle any using declarations or other typedefs.
        typename = _prettify_typename(val.type)
        if not typename:
            return None
        without_generics = _remove_generics(typename)
        lookup_name = _remove_std_prefix(without_generics)
        if lookup_name in self.lookup:
            return self.lookup[lookup_name](val)
        return None

_rogue_custom_printer_name = "rogue_pretty_printer"

# These are called for every binary object file, which could be thousands in
# certain pathological cases. Limit our pretty printers to the progspace.
def _register_libcxx_printers(event):
    progspace = event.new_objfile.progspace
    # It would be ideal to get the endianness at print time, but
    # gdb.execute clears gdb's internal wrap buffer, removing any values
    # already generated as part of a larger data structure, and there is
    # no python api to get the endianness. Mixed-endianness debugging
    # rare enough that this workaround should be adequate.
    _libcpp_big_endian = "big endian" in gdb.execute("show endian", to_string=True)

    if not getattr(progspace, _rogue_custom_printer_name, False):
        print("Loading libc++ pretty-printers.")
        gdb.printing.register_pretty_printer(
            progspace, RogueCustomPrettyPrinter(_rogue_custom_printer_name)
        )
        setattr(progspace, _rogue_custom_printer_name, True)


def _unregister_libcxx_printers(event):
    progspace = event.progspace
    if getattr(progspace, _rogue_custom_printer_name, False):
        for printer in progspace.pretty_printers:
            if getattr(printer, "name", "none") == _rogue_custom_printer_name:
                progspace.pretty_printers.remove(printer)
                setattr(progspace, _rogue_custom_printer_name, False)
                break


def register_libcxx_printer_loader():
    """Register event handlers to load libc++ pretty-printers."""
    gdb.events.new_objfile.connect(_register_libcxx_printers)
    gdb.events.clear_objfiles.connect(_unregister_libcxx_printers)
