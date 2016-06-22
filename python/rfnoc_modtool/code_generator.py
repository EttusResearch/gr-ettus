""" A code generator (needed by ModToolAdd) """

from templates import Templates
import Cheetah.Template
from util_functions import str_to_fancyc_comment
from util_functions import str_to_python_comment
from util_functions import strip_default_values
from util_functions import strip_arg_types
from util_functions import strip_arg_types_grc

class GRMTemplate(Cheetah.Template.Template):
    """ An extended template class """
    def __init__(self, src, searchList):
        self.grtypelist = {
                'sync': 'sync_block',
                'sink': 'sync_block',
                'source': 'sync_block',
                'decimator': 'sync_decimator',
                'interpolator': 'sync_interpolator',
                'general': 'block',
                'tagged_stream': 'tagged_stream_block',
                'hier': 'hier_block2',
                'rfnoc':'ettus::rfnoc_block',
                'noblock': ''}
        searchList['str_to_fancyc_comment'] = str_to_fancyc_comment
        searchList['str_to_python_comment'] = str_to_python_comment
        searchList['strip_default_values'] = strip_default_values
        searchList['strip_arg_types'] = strip_arg_types
        searchList['strip_arg_types_grc'] = strip_arg_types_grc
        Cheetah.Template.Template.__init__(self, src, searchList=searchList)
        self.grblocktype = self.grtypelist[searchList['blocktype']]
        if searchList['is_component']:
            self.include_dir_prefix = "gnuradio/" + searchList['modname']
        else:
            self.include_dir_prefix = searchList['modname']

def get_template(tpl_id, **kwargs):
    """ Return the template given by tpl_id, parsed through Cheetah """
    return str(GRMTemplate(Templates[tpl_id], searchList=kwargs))
