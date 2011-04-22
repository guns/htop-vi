# Copyright (c) 2011 Sung Pae <self@sungpae.com>
# Distributed under the MIT license.
# http://www.opensource.org/licenses/mit-license.php

task :default => :configure

desc 'Configure htop'
task :configure do
  configure = File.expand_path 'configure'
  system './autogen.sh' unless File.executable? configure

  opts = %W[
    --prefix=#{ENV['PREFIX'] || '/opt/htop'}
    --enable-openvz
    --enable-vserver
  ]

  puts ([configure] + opts).join(' ')
  system configure, *opts
end
