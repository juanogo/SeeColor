function M = img_blur(M,sigma)
% Blures gray scale image with gaussian kernel. 

if nargin==1
    sigma = 1.5;
end

csize = 6*sigma;

shift = (csize - 1)/2;

h = FSPECIAL('gaussian',csize,sigma);

M = conv2(double(M),h);
M = M(round(1+shift:end-shift),round(1+shift:end-shift));
M = uint8(M);
