/*
 *   This file is generated by Tapenade 3.14 (r7259) from a file "ba.c".
 *   To reproduce such a generation you can use Tapenade CLI
 *   (can be downloaded from http://www-sop.inria.fr/tropics/tapenade/downloading.html)
 *   
 *   After installing use the next command to generate a file:
 *
 *      tapenade -b -o ba_tapenade -head "compute_reproj_error(err)/(cam X) compute_zach_weight_error(err)/(w)" ba.c
 *
 *   This will produce a file "ba_tapenade_b.c" which content will be the same as the content of this file,
 *   except this header. Moreover a log-file "ba_tapenade_b.msg" will be produced.
 *
 *   NOTE: this code is wrong and won't work. REPAIRED SOURCE IS STORED IN THE FILE "ba_b.c".
 *         You can use diff tool to figure out what changes was performed to fix the code.
 *   
 *   NOTE: you can also use Tapenade web server (http://tapenade.inria.fr:8080/tapenade/index.jsp)
 *         for generating but the result can be slightly different.
 */

 /*        Generated by TAPENADE     (INRIA, Ecuador team)
    Tapenade 3.14 (r7259) - 18 Jan 2019 09:35
*/
#include <adBuffer.h>

/*
  Differentiation of sqsum in reverse (adjoint) mode:
   gradient     of useful results: *x sqsum
   with respect to varying inputs: *x
   Plus diff mem management of: x:in

 ===================================================================== 
                                UTILS                                  
 ===================================================================== */
void sqsum_b(int n, const double *x, double *xb, double sqsumb) {
    int i;
    double res = 0;
    double resb = 0.0;
    double sqsum;
    resb = sqsumb;
    for (i = n-1; i > -1; --i)
        xb[i] = xb[i] + 2*x[i]*resb;
}

/* ===================================================================== 
                                UTILS                                  
 ===================================================================== */
double sqsum_nodiff(int n, const double *x) {
    int i;
    double res = 0;
    for (i = 0; i < n; ++i)
        res = res + x[i]*x[i];
    return res;
}

/*
  Differentiation of cross in reverse (adjoint) mode:
   gradient     of useful results: *out *a *b
   with respect to varying inputs: *a *b
   Plus diff mem management of: out:in a:in b:in
*/
void cross_b(const double *a, double *ab, const double *b, double *bb, double 
        *out, double *outb) {
    ab[0] = ab[0] + b[1]*outb[2];
    bb[1] = bb[1] + a[0]*outb[2];
    ab[1] = ab[1] - b[0]*outb[2];
    bb[0] = bb[0] - a[1]*outb[2];
    outb[2] = 0.0;
    ab[2] = ab[2] + b[0]*outb[1];
    bb[0] = bb[0] + a[2]*outb[1];
    ab[0] = ab[0] - b[2]*outb[1];
    bb[2] = bb[2] - a[0]*outb[1];
    outb[1] = 0.0;
    ab[1] = ab[1] + b[2]*outb[0];
    bb[2] = bb[2] + a[1]*outb[0];
    ab[2] = ab[2] - b[1]*outb[0];
    bb[1] = bb[1] - a[2]*outb[0];
}

void cross_nodiff(const double *a, const double *b, double *out) {
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}

/*
  Differentiation of rodrigues_rotate_point in reverse (adjoint) mode:
   gradient     of useful results: *rot *rotatedPt
   with respect to varying inputs: *rot *pt
   Plus diff mem management of: rot:in rotatedPt:in pt:in

 ===================================================================== 
                               MAIN LOGIC                              
 ===================================================================== */
// rot: 3 rotation parameters
// pt: 3 point to be rotated
// rotatedPt: 3 rotated point
// this is an efficient evaluation (part of
// the Ceres implementation)
// easy to understand calculation in matlab:
//  theta = sqrt(sum(w. ^ 2));
//  n = w / theta;
//  n_x = au_cross_matrix(n);
//  R = eye(3) + n_x*sin(theta) + n_x*n_x*(1 - cos(theta));    
void rodrigues_rotate_point_b(const double *rot, double *rotb, const double *
        pt, double *ptb, double *rotatedPt, double *rotatedPtb) {
    int i;
    double sqtheta;
    double sqthetab;
    int ii1;
    sqtheta = sqsum_nodiff(3, rot);
    if (sqtheta != 0) {
        double theta, costheta, sintheta, theta_inverse;
        double w[3], w_cross_pt[3], tmp;
        double tempb;
        theta = sqrt(sqtheta);
        costheta = cos(theta);
        sintheta = sin(theta);
        theta_inverse = 1.0/theta;
        double thetab, costhetab, sinthetab, theta_inverseb;
        double wb[3], w_cross_ptb[3], tmpb;
        for (i = 0; i < 3; ++i)
            w[i] = rot[i]*theta_inverse;
        cross_nodiff(w, pt, w_cross_pt);
        tmp = (w[0]*pt[0]+w[1]*pt[1]+w[2]*pt[2])*(1.-costheta);
        *ptb = 0.0;
        for (ii1 = 0; ii1 < 3; ++ii1)
            w_cross_ptb[ii1] = 0.0;
        for (ii1 = 0; ii1 < 3; ++ii1)
            wb[ii1] = 0.0;
        costhetab = 0.0;
        tmpb = 0.0;
        sinthetab = 0.0;
        for (i = 2; i > -1; --i) {
            ptb[i] = ptb[i] + costheta*rotatedPtb[i];
            costhetab = costhetab + pt[i]*rotatedPtb[i];
            w_cross_ptb[i] = w_cross_ptb[i] + sintheta*rotatedPtb[i];
            sinthetab = sinthetab + w_cross_pt[i]*rotatedPtb[i];
            wb[i] = wb[i] + tmp*rotatedPtb[i];
            tmpb = tmpb + w[i]*rotatedPtb[i];
            rotatedPtb[i] = 0.0;
        }
        tempb = (1.-costheta)*tmpb;
        wb[0] = wb[0] + pt[0]*tempb;
        ptb[0] = ptb[0] + w[0]*tempb;
        wb[1] = wb[1] + pt[1]*tempb;
        ptb[1] = ptb[1] + w[1]*tempb;
        wb[2] = wb[2] + pt[2]*tempb;
        ptb[2] = ptb[2] + w[2]*tempb;
        costhetab = costhetab - (w[0]*pt[0]+w[1]*pt[1]+w[2]*pt[2])*tmpb;
        cross_b(w, wb, pt, ptb, w_cross_pt, w_cross_ptb);
        theta_inverseb = 0.0;
        for (i = 2; i > -1; --i) {
            rotb[i] = rotb[i] + theta_inverse*wb[i];
            theta_inverseb = theta_inverseb + rot[i]*wb[i];
            wb[i] = 0.0;
        }
        thetab = cos(theta)*sinthetab - sin(theta)*costhetab - theta_inverseb/
            (theta*theta);
        if (sqtheta == 0.0)
            sqthetab = 0.0;
        else
            sqthetab = thetab/(2.0*sqrt(sqtheta));
    } else {
        {
          double rot_cross_pt[3];
          double rot_cross_ptb[3];
          *ptb = 0.0;
          for (ii1 = 0; ii1 < 3; ++ii1)
              rot_cross_ptb[ii1] = 0.0;
          for (i = 2; i > -1; --i) {
              ptb[i] = ptb[i] + rotatedPtb[i];
              rot_cross_ptb[i] = rot_cross_ptb[i] + rotatedPtb[i];
              rotatedPtb[i] = 0.0;
          }
          cross_b(rot, rotb, pt, ptb, rot_cross_pt, rot_cross_ptb);
        }
        sqthetab = 0.0;
    }
    sqsum_b(3, rot, rotb, sqthetab);
}

/* ===================================================================== 
                               MAIN LOGIC                              
 ===================================================================== */
// rot: 3 rotation parameters
// pt: 3 point to be rotated
// rotatedPt: 3 rotated point
// this is an efficient evaluation (part of
// the Ceres implementation)
// easy to understand calculation in matlab:
//  theta = sqrt(sum(w. ^ 2));
//  n = w / theta;
//  n_x = au_cross_matrix(n);
//  R = eye(3) + n_x*sin(theta) + n_x*n_x*(1 - cos(theta));    
void rodrigues_rotate_point_nodiff(const double *rot, const double *pt, double
        *rotatedPt) {
    int i;
    double sqtheta;
    sqtheta = sqsum_nodiff(3, rot);
    if (sqtheta != 0) {
        double theta, costheta, sintheta, theta_inverse;
        double w[3], w_cross_pt[3], tmp;
        theta = sqrt(sqtheta);
        costheta = cos(theta);
        sintheta = sin(theta);
        theta_inverse = 1.0/theta;
        for (i = 0; i < 3; ++i)
            w[i] = rot[i]*theta_inverse;
        cross_nodiff(w, pt, w_cross_pt);
        tmp = (w[0]*pt[0]+w[1]*pt[1]+w[2]*pt[2])*(1.-costheta);
        for (i = 0; i < 3; ++i)
            rotatedPt[i] = pt[i]*costheta + w_cross_pt[i]*sintheta + w[i]*tmp;
    } else {
        double rot_cross_pt[3];
        cross_nodiff(rot, pt, rot_cross_pt);
        for (i = 0; i < 3; ++i)
            rotatedPt[i] = pt[i] + rot_cross_pt[i];
    }
}

/*
  Differentiation of radial_distort in reverse (adjoint) mode:
   gradient     of useful results: *rad_params *proj
   with respect to varying inputs: *rad_params *proj
   Plus diff mem management of: rad_params:in proj:in
*/
void radial_distort_b(const double *rad_params, double *rad_paramsb, double *
        proj, double *projb) {
    double rsq, L;
    double rsqb, Lb;
    rsq = sqsum_nodiff(2, proj);
    L = 1. + rad_params[0]*rsq + rad_params[1]*rsq*rsq;
    pushReal8(proj[0]);
    proj[0] = proj[0]*L;
    Lb = proj[1]*projb[1];
    projb[1] = L*projb[1];
    popReal8(&(proj[0]));
    Lb = Lb + proj[0]*projb[0];
    projb[0] = L*projb[0];
    rad_paramsb[0] = rad_paramsb[0] + rsq*Lb;
    rsqb = (rad_params[1]*2*rsq+rad_params[0])*Lb;
    rad_paramsb[1] = rad_paramsb[1] + rsq*rsq*Lb;
    sqsum_b(2, proj, projb, rsqb);
}

void radial_distort_nodiff(const double *rad_params, double *proj) {
    double rsq, L;
    rsq = sqsum_nodiff(2, proj);
    L = 1. + rad_params[0]*rsq + rad_params[1]*rsq*rsq;
    proj[0] = proj[0]*L;
    proj[1] = proj[1]*L;
}

/*
  Differentiation of project in reverse (adjoint) mode:
   gradient     of useful results: *proj
   with respect to varying inputs: *cam *X
   Plus diff mem management of: cam:in X:in proj:in
*/
void project_b(const double *cam, double *camb, const double *X, double *Xb, 
        double *proj, double *projb) {
    double *C = &(cam[3]);
    double *Cb = &(camb[3]);
    double Xo[3], Xcam[3];
    double Xob[3], Xcamb[3];
    int ii1;
    double tempb;
    double tempb0;
    Xo[0] = X[0] - C[0];
    Xo[1] = X[1] - C[1];
    Xo[2] = X[2] - C[2];
    rodrigues_rotate_point_nodiff(&(cam[0]), Xo, Xcam);
    proj[0] = Xcam[0]/Xcam[2];
    proj[1] = Xcam[1]/Xcam[2];
    pushReal8(*proj);
    radial_distort_nodiff(&(cam[9]), proj);
    pushReal8(proj[0]);
    proj[0] = proj[0]*cam[6] + cam[7];
    camb[6] = camb[6] + proj[1]*projb[1];
    camb[8] = camb[8] + projb[1];
    projb[1] = cam[6]*projb[1];
    popReal8(&(proj[0]));
    camb[6] = camb[6] + proj[0]*projb[0];
    camb[7] = camb[7] + projb[0];
    projb[0] = cam[6]*projb[0];
    popReal8(proj);
    radial_distort_b(&(cam[9]), &(camb[9]), proj, projb);
    for (ii1 = 0; ii1 < 3; ++ii1)
        Xcamb[ii1] = 0.0;
    tempb = projb[1]/Xcam[2];
    Xcamb[1] = Xcamb[1] + tempb;
    Xcamb[2] = Xcamb[2] - Xcam[1]*tempb/Xcam[2];
    projb[1] = 0.0;
    tempb0 = projb[0]/Xcam[2];
    Xcamb[0] = Xcamb[0] + tempb0;
    Xcamb[2] = Xcamb[2] - Xcam[0]*tempb0/Xcam[2];
    rodrigues_rotate_point_b(&(cam[0]), &(camb[0]), Xo, Xob, Xcam, Xcamb);
    Xb[2] = Xb[2] + Xob[2];
    Cb[2] = Cb[2] - Xob[2];
    Xob[2] = 0.0;
    Xb[1] = Xb[1] + Xob[1];
    Cb[1] = Cb[1] - Xob[1];
    Xob[1] = 0.0;
    Xb[0] = Xb[0] + Xob[0];
    Cb[0] = Cb[0] - Xob[0];
}

void project_nodiff(const double *cam, const double *X, double *proj) {
    const double *C = &(cam[3]);
    double Xo[3], Xcam[3];
    Xo[0] = X[0] - C[0];
    Xo[1] = X[1] - C[1];
    Xo[2] = X[2] - C[2];
    rodrigues_rotate_point_nodiff(&(cam[0]), Xo, Xcam);
    proj[0] = Xcam[0]/Xcam[2];
    proj[1] = Xcam[1]/Xcam[2];
    radial_distort_nodiff(&(cam[9]), proj);
    proj[0] = proj[0]*cam[6] + cam[7];
    proj[1] = proj[1]*cam[6] + cam[8];
}

/*
  Differentiation of compute_reproj_error in reverse (adjoint) mode:
   gradient     of useful results: *err
   with respect to varying inputs: *err *w *cam *X
   RW status of diff variables: *err:in-out *w:out *cam:out *X:out
   Plus diff mem management of: err:in w:in cam:in X:in
*/
// cam: 11 camera in format [r1 r2 r3 C1 C2 C3 f u0 v0 k1 k2]
//            r1, r2, r3 are angle - axis rotation parameters(Rodrigues)
//            [C1 C2 C3]' is the camera center
//            f is the focal length in pixels
//            [u0 v0]' is the principal point
//            k1, k2 are radial distortion parameters
// X: 3 point
// feats: 2 feature (x,y coordinates)
// reproj_err: 2
// projection function: 
// Xcam = R * (X - C)
// distorted = radial_distort(projective2euclidean(Xcam), radial_parameters)
// proj = distorted * f + principal_point
// err = sqsum(proj - measurement)
void compute_reproj_error_b(const double *cam, double *camb, const double *X, 
        double *Xb, const double *w, double *wb, const double *feat, double *
        err, double *errb) {
    double proj[2];
    double projb[2];
    int ii1;
    pushReal8Array(proj, 2);
    project_nodiff(cam, X, proj);
    for (ii1 = 0; ii1 < 2; ++ii1)
        projb[ii1] = 0.0;
    *wb = (proj[1]-feat[1])*errb[1];
    projb[1] = projb[1] + (*w)*errb[1];
    errb[1] = 0.0;
    *wb = *wb + (proj[0]-feat[0])*errb[0];
    projb[0] = projb[0] + (*w)*errb[0];
    errb[0] = 0.0;
    popReal8Array(proj, 2);
    project_b(cam, camb, X, Xb, proj, projb);
}

/*
  Differentiation of compute_zach_weight_error in reverse (adjoint) mode:
   gradient     of useful results: *err
   with respect to varying inputs: *err *w
   RW status of diff variables: *err:in-out *w:out
   Plus diff mem management of: err:in w:in
*/
void compute_zach_weight_error_b(const double *w, double *wb, double *err, 
        double *errb) {
    *wb = -(2*(*w)*(*errb));
    *errb = 0.0;
}
