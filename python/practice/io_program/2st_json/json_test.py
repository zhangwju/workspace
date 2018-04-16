#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-

import json
obj = dict(name='小明', age=20)
s = json.dumps(obj)#.encode()
print(s)
