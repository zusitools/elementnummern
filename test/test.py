#!/usr/bin/env python3

import subprocess
import unittest
import os

class TestElementnummern(unittest.TestCase):
  def run_elementnummern(self, args, line_filter):
    out = subprocess.check_output(['../build/elementnummern'] + args, env={'ZUSI3_DATAPATH': os.path.join(os.getcwd(), '')}, text=True).splitlines()
    return [line for line in out if line_filter in line]

  def test_manuelle_register(self):
    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '1'], "Register")
    self.assertEqual([r"(Manuelles) Register Nr. 1: Routes\ModulA.st3, Register Nr. 1"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '42'], "Register")
    self.assertEqual([r"(Manuelles) Register Nr. 42: Routes\ModulA.st3, Register Nr. 42"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '43'], "Register")
    self.assertEqual([r"(Manuelles) Register Nr. 43: Routes\ModulB.st3, Register Nr. 1"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '84'], "Register")
    self.assertEqual([r"(Manuelles) Register Nr. 84: Routes\ModulB.st3, Register Nr. 42"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '85'], "Register")
    self.assertEqual([], out)

  def test_automatische_register(self):
    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '5002'], "Register")
    self.assertEqual([r"(Automatisches) Register Nr. 5002: Routes\ModulA.st3, Register Nr. 5001"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '5004'], "Register")
    self.assertEqual([r"(Automatisches) Register Nr. 5004: Routes\ModulB.st3, Register Nr. 5001"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '5005'], "Register")
    self.assertEqual([], out)

  def test_elementnummern(self):
    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '3'], "Streckenelement")
    self.assertEqual([r"Streckenelement Nr. 3: Routes\ModulA.st3, Streckenelement Nr. 2"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '6'], "Streckenelement")
    self.assertEqual([r"Streckenelement Nr. 6: Routes\ModulA.st3, Streckenelement Nr. 5"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '8'], "Streckenelement")
    self.assertEqual([r"Streckenelement Nr. 8: Routes\ModulB.st3, Streckenelement Nr. 1"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '12'], "Streckenelement")
    self.assertEqual([r"Streckenelement Nr. 12: Routes\ModulB.st3, Streckenelement Nr. 5"], out)

    out = self.run_elementnummern(['Timetables/Fahrplan.fpn', '13'], "Streckenelement")
    self.assertEqual([], out)

if __name__ == '__main__':
  suite = unittest.TestLoader().loadTestsFromTestCase(TestElementnummern)
  unittest.TextTestRunner(verbosity=2).run(suite)
