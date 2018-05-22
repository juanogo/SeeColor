function ND=calibrateKinect(D)
%% This is a function to calibrate the depth from the Kinect based on the
% calibration method done by Juan Gomez, Université de Genève.
% Input: Depth Image.
% Output: Calibrated Depth Image.
% Nx, Ny, Mx, My were calculated using a ANN (attached) according to Juan Gomez Method:
% https://jivp-eurasipjournals.springeropen.com/articles/10.1186/1687-5281-2013-41


D = double(D);

Nx =    [0.0806 0.0052 0.9967]';
Ny =    [0.0828 0.0003 0.9966]';
Mx =  1.0e+003 * [0.3020 1.8064 0.0178];
My =  1.0e+003 * [0.1686 1.8064 0.0182];

[X,Y] = meshgrid(1:640, 1:480);
Sx =((Mx*Nx)-X*Nx(1)-D*Nx(2))/Nx(3);
Sy =((My*Ny)-Y*Ny(1)-D*Ny(2))/Ny(3);

Sx=Sx.*(D>0);Sy=Sy.*(D>0);

 ND=zeros(size(D));

 IND2=Y(:)+ 480 * (X(:)-1);
 val=D(IND2)>0;
 IND2(val==0)=[];

 temp1=round(Y(:)+Sy(:));
 temp2=round(X(:)+Sx(:));
 temp1(val==0)=[];
 temp2(val==0)=[];

 val1=temp1<=480;
 val2=temp2<=640;

 IND2(val1.*val2==0)=[];
 temp1(val1.*val2==0)=[];
 temp2(val1.*val2==0)=[];
 IND=temp1+ 480 * (temp2-1); 

 ND(IND)=D(IND2);
 
end
