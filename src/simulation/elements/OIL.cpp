#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_OIL PT_OIL 3

float blueOffsetOil = 121.0f;

Element_OIL::Element_OIL()
{
	Identifier = "DEFAULT_PT_OIL";
	Name = "OIL";
	Colour = PIXPACK(0x7A7A0F);
	MenuVisible = 1;
	MenuSection = SC_ORGANIC;
	Enabled = 1;
	
	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;
	
	Weight = 20;
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 42;
	Description = "Flammable, boils into a gasseous form.";
	
	State = ST_GAS;
	Properties = TYPE_GAS | PROP_NEUTPASS;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_OIL::update;
	
}

//#TPT-Directive ElementHeader Element_OIL static int update(UPDATE_FUNC_ARGS)
int Element_OIL::update(UPDATE_FUNC_ARGS){
	parts[i].element = 1;
	int r, rx, ry;
	for (rx = -2; rx<3; rx++)
		for (ry = -2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry)){
		r = pmap[y + ry][x + rx];
		if (!r)
			continue;
		chemDat H = parts[i].hydrogen;
		chemDat C = parts[i].carbon;
		chemDat Cl = parts[i].chlorine;
		chemDat alc = parts[i].alcohol;
		chemDat bond = parts[i].bond;
		int bondAmt;
		int bonds = H + (Cl > 0) + (alc > 0);
		if (C < 1)C = rand() % 49 + 11;
		if (C > 1){
			if (bonds>2 * C + 2)H = (2 * C + 2) - alc - Cl;
			else{
				H = rand() % (2 * C - 3) - alc - Cl;
				
			}
		}
		else{
			if (bonds < 4 || bonds > 4)H = bonds - alc - Cl;
		}
		if (H == 2 * C + 2)bondAmt = 1;
		else if (H == 2 * C - 2)bondAmt = 3;
		else bondAmt = 2;
		if (bond > 0 && bondAmt == 1)bond = 0;
		else if (bond == 0 && bondAmt > 1){
			if (C % 2 == 0)bond = rand() % (C / 2) + 1;
			else bond = rand() % ((C + 1) / 2) + 1;
		}
		if ((bonds > 2 * C + 2 && C > 1 && bondAmt == 1) || (bonds > 2 * C && C > 1 && bondAmt == 2) || (bonds > 2 * C - 2 && bondAmt == 3) || (C == 1 && bonds > 4))H--;
		if (alc > 0)sim->part_change_type(i, x, y, PT_ALCO);
		//

		//
		parts[i].energy = (parts[i].temp - R_TEMP) - rand() % 25 - 12;
		if ((r & 0xFF) == PT_H2 && bonds + parts[r >> 8].life <= 2 * C + 2 && parts[i].energy >= 100 * C && parts[i].energy <= 200 * C){
			H += parts[r >> 8].life;
			parts[i].energy -= rand() % 12 + 12;
			sim->kill_part(r >> 8);
		}
		if ((r & 0xFF) == PT_CRBN && bonds < 2 * C + 2 && bondAmt > 1 && C < 59 && parts[i].energy >= 100 * C && parts[i].energy <= 150 * C){
			C++;
			parts[i].energy -= rand() % 24 + 24;
			sim->kill_part(r >> 8);
			for (int ch = 0; ch <= bondAmt; ch++){
				sim->pv[y / CELL][x / CELL] += 0.05f;
				H--;
				parts[i].energy += rand() % 12 + 12;
			}
			if (C % 2 == 0 && bondAmt > 1)bond = rand() % (C / 2) + 1;
			else if (C % 2 != 0 && bondAmt > 1) bond = rand() % ((C + 1) / 2) + 1;
		}
		if ((r & 0xFF) == PT_CLRN && bonds < 2 * C + 2 && parts[i].energy >= 100 * C && parts[i].energy <= 200 * C && Cl == 0){
			Cl = rand() % (C / 2) + 1;
			H--;
			parts[i].energy -= rand() % 12 + 12;
			sim->kill_part(r >> 8);
		}
		if ((r & 0xFF) == PT_HXDE && bonds < 2 * C + 2 && parts[i].energy >= 100 * C && parts[i].energy <= 200 * C && alc == 0){
			alc = rand() % (C / 2) + 1;
			H--;
			sim->part_change_type(i, x, y, PT_ALCO);
			parts[i].energy -= rand() % 12 + 12;
			sim->kill_part(r >> 8);
		}
		if ((parts[i].temp >= 109.15f && C <= 10) || (parts[i].temp >= 300.0f && C > 10)){
			sim->part_change_type(i, x, y, PT_HCBN);
		}
		parts[i].hydrogen = H;
		parts[i].carbon = C;
		parts[i].chlorine = Cl;
		parts[i].alcohol = alc;
		parts[i].bond = bond;
		parts[i].bonds = bonds;
		if (blueOffsetOil < 255.0f)blueOffsetOil = 10 * C;
		else blueOffsetOil = 255.0f;
			}
	return 0;
}

//#TPT-Directive ElementHeader Element_OIL static int graphics(GRAPHICS_FUNC_ARGS)
int Element_OIL::graphics(GRAPHICS_FUNC_ARGS){
	*colr = 237.0f;
	*colg = 235.0f;
	*colb = blueOffsetOil;

	*firea = 50.0f;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	return 0;
}

Element_OIL::~Element_OIL() {}
