/*****************************************************************************
 *  \author 
 *  	Erwin Aertbelien, Div. PMA, Dep. of Mech. Eng., K.U.Leuven
 *
 *  \version 
 *		ORO_Geometry V0.2
 *
 *	\par History
 *		- $log$
 *
 *	\par Release
 *		$Id: path_circle.cpp,v 1.1.1.1.2.5 2003/07/24 13:26:15 psoetens Exp $
 *		$Name:  $ 
 ****************************************************************************/


#include "geometry/path_circle.h"
#include "geometry/error.h"

#ifdef USE_NAMESPACE
namespace ORO_Geometry {
#endif



Path_Circle::Path_Circle(const Frame& F_base_start,
			const Vector& _V_base_center,
			const Vector& V_base_p,
			const Rotation& R_base_end,
			double alpha,
			Orientation* _orient,
			double _eqradius, 
            bool _aggregate) :
				orient(_orient) ,
				eqradius(_eqradius),
                aggregate(_aggregate)
			{
					F_base_center.p = _V_base_center;
					orient->SetStartEnd(F_base_start.M,R_base_end);
					double oalpha = orient->Angle();

					Vector x(F_base_start.p - F_base_center.p);
					radius = x.Normalize();
#ifdef HAVE_EXCEPTIONS
					if (radius < epsilon) throw Error_MotionPlanning_Circle_ToSmall();
#endif
					Vector tmpv(V_base_p-F_base_center.p);
					tmpv.Normalize();
					Vector z( x * tmpv);
#ifdef HAVE_EXCEPTIONS
                    double n = z.Normalize();
				    if (n < epsilon) throw Error_MotionPlanning_Circle_No_Plane();
#endif
					F_base_center.M = Rotation(x,z*x,z);
					double dist = alpha*radius;
					// See what has the slowest eq. motion, and adapt 
					// the other to this slower motion
					// use eqradius to transform between rot and transl.
					// the same as for lineair motion
					if (oalpha*eqradius > dist) {
						// rotational_interpolation is the limitation
						pathlength = oalpha*eqradius;
						scalerot   = 1/eqradius;
						scalelin   = dist/pathlength;
					} else {
						// translation is the limitation
						pathlength = dist;
						scalerot   = oalpha/pathlength;
						scalelin   = 1;
					}
			}

				
		
double Path_Circle::LengthToS(double length) {
	return length/scalelin;
}


double Path_Circle::PathLength() {
	return pathlength;
}

Frame Path_Circle::Pos(double s) const {
	double p = s*scalelin / radius;
	return Frame(orient->Pos(s*scalerot), 
		         F_base_center*Vector(radius*cos(p),radius*sin(p),0)
		   );
				   
}

Twist Path_Circle::Vel(double s,double sd) const {
	double p = s*scalelin  / radius;
	double v = sd*scalelin / radius;
	return Twist( F_base_center.M*Vector(-radius*sin(p)*v,radius*cos(p)*v,0), 
		          orient->Vel(s*scalerot,sd*scalerot) 
		   );
}

Twist Path_Circle::Acc(double s,double sd,double sdd) const {
	double p = s*scalelin / radius;
	double cp = cos(p);
	double sp = sin(p);
	double v = sd*scalelin / radius;
	double a = sdd*scalelin / radius;
	return Twist( F_base_center.M*Vector(
						-radius*cp*v*v  -  radius*sp*a,
						-radius*sp*v*v  +  radius*cp*a,
						0
					  ), 
		          orient->Acc(s*scalerot,sd*scalerot,sdd*scalerot) 
		   );
}

Path* Path_Circle::Clone() {
	return new Path_Circle(
		Pos(0),
		F_base_center.p,
		F_base_center.M.UnitY(),
		orient->Pos(pathlength*scalerot),
		pathlength*scalelin/radius/deg2rad,
		orient->Clone(),
		eqradius,
        aggregate
	);
}

Path_Circle::~Path_Circle() {
    if (aggregate)
        delete orient;
}



#if HAVE_IOSTREAM
void Path_Circle::Write(ostream& os) {
	os << "CIRCLE[ ";
	os << "  " << Pos(0) << endl;
	os << "  " << F_base_center.p << endl;
	os << "  " << F_base_center.M.UnitY() << endl;	
	os << "  " << orient->Pos(pathlength*scalerot) << endl;
	os << "  " << pathlength*scalelin/radius/deg2rad << endl;
	os << "  ";orient->Write(os);
	os << "  " << eqradius;
	os << "]"<< endl;
}
#endif


#ifdef USE_NAMESPACE
}
#endif

