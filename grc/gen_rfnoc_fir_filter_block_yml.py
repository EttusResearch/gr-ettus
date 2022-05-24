"""
Copyright 2022 Ettus Research, A National Instruments Brand.

This is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street,
Boston, MA 02110-1301, USA.
"""

import sys

MAX_NUM_CHANNELS = 16

MAIN_TMPL = """\
id: ettus_rfnoc_fir_filter
label: RFNoC FIR Filter Block

parameters:
- id: num_chans
  label: 'Num Channels'
  dtype: int
  default: 1
  options: [ ${", ".join([str(n) for n in range(1, max_num_chans+1)])} ]
- id: block_args
  label: Block Args
  dtype: string
  default: ""
- id: device_select
  label: Device Select
  dtype: int
  default: -1
- id: instance_index
  label: Instance Select
  dtype: int
  default: -1
${coeffs_params}
inputs:
- domain: rfnoc
  dtype: 'sc16'
  multiplicity: ${'$'}{num_chans}

outputs:
- domain: rfnoc
  dtype: 'sc16'
  multiplicity: ${'$'}{num_chans}

templates:
  imports: |-
    import ettus
    from gnuradio import uhd
  make: |-
    ettus.rfnoc_fir_filter(
        self.rfnoc_graph,
        uhd.device_addr(${'$'}{block_args}),
        ${'$'}{device_select},
        ${'$'}{instance_index})
${init_params}  callbacks:
${callback_params}

documentation: |-
  RFNoC FIR Filter Block:
  Applies a FIR filter with user defined coefficients to the input sample 
  stream.

  Number of Channels:
  Number of channels / streams to use with the RFNoC FIR filter block.
  Note, this is defined by the RFNoC FIR Filter Block's FPGA build 
  parameters and GNU Radio Companion is not aware of this value. An error
  will occur at runtime when connecting blocks if the number of channels is
  too large.

  Coefficient Type:
  Choice between floating point or integer coefficients.
  Floating point coefficients must be within range [-1.0, 1.0].
  Integer coefficients must be within range [-32768, 32767].

  Coefficients:
  Array of coefficients. Number of elements in the array implicitly sets
  the filter length, and must be less than or equal to the maximum number
  of coefficients supported by the block.

file_format: 1
"""

COEFFS_PARAM = """- id: coeffs_type${n}
  label: 'Ch${n}: Coeffs Type'
  dtype: enum
  options: ['float', 'integer']
  default: 'float'
  hide: ${'$'}{ 'part' if num_chans > ${n} else 'all' }
- id: coeffs_float${n}
  label: 'Ch${n}: Coefficients'
  dtype: float_vector
  default: [1.0]
  hide: ${'$'}{ 'none' if coeffs_type${n} == 'float' and num_chans > ${n} else 'all' }
- id: coeffs_int${n}
  label: 'Ch${n}: Coefficients'
  dtype: int_vector
  default: [32767]
  hide: ${'$'}{ 'none' if coeffs_type${n} == 'integer' and num_chans > ${n} else 'all' }
"""

INIT_PARAM = """    ${'%'} if coeffs_type${n} == "float" and context.get('num_chans')() > ${n}:
    self.${'$'}{id}.set_coefficients_float(${'$'}{coeffs_float${n}}, ${n})
    ${'%'} endif
    ${'%'} if coeffs_type${n} == "integer" and context.get('num_chans')() > ${n}:
    self.${'$'}{id}.set_coefficients_int(${'$'}{coeffs_int${n}}, ${n})
    ${'%'} endif
"""

CALLBACKS_PARAM = """  - |
    ${'%'} if coeffs_type${n} == "float" and context.get('num_chans')() > ${n}:
    self.${'$'}{id}.set_coefficients_float(${'$'}{coeffs_float${n}}, ${n})
    ${'%'} endif
  - |
    ${'%'} if coeffs_type${n} == "integer" and context.get('num_chans')() > ${n}:
    self.${'$'}{id}.set_coefficients_int(${'$'}{coeffs_int${n}}, ${n})
    ${'%'} endif
"""

def parse_tmpl(_tmpl, **kwargs):
    """ Render _tmpl using the kwargs and write to file """
    from mako.template import Template

    block_template = Template(_tmpl)
    return str(block_template.render(**kwargs))

if __name__ == '__main__':
    file = sys.argv[1];
    coeffs_params = ''.join([
        parse_tmpl(COEFFS_PARAM, n=n)
        for n in range(MAX_NUM_CHANNELS)
    ])
    init_params = ''.join([
        parse_tmpl(INIT_PARAM, n=n)
        for n in range(MAX_NUM_CHANNELS)
    ])
    callback_params = ''.join([
        parse_tmpl(CALLBACKS_PARAM, n=n)
        for n in range(MAX_NUM_CHANNELS)
    ])
    open(file, 'w').write(
        parse_tmpl(
            MAIN_TMPL,
            max_num_chans=MAX_NUM_CHANNELS,
            coeffs_params=coeffs_params,
            init_params=init_params,
            callback_params=callback_params
        )
    )
