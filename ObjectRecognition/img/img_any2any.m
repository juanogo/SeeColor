% mkdir(dirname2);
ext1 = 'png';
ext2 = 'png';

files = dir([dirname '*.' ext1]);
for i = 1:length(files)
    clc; disp([i length(files)]);
    img = imread([dirname files(i).name]);

    if size(img,1)~=420
    
        %img = imresize(img,0.25);
        %img = img(1:420,1:640,:);
        img = img(1:240,1:320,:);
        [pathname name ext] = fileparts(files(i).name);
        imwrite(img,[dirname2 name '.' ext2]);
    end
   
end
