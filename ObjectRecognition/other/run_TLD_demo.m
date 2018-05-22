application

addpath(genpath('.')); init_workspace; 

% Input
opt.source          = struct('camera',0,'input','_input/','bb0',[]);
opt.debug           = 0;
opt.name            = '_snapshots/'; mkdir(opt.name); % saves s snapshot every 100th frames

% Parameters
opt.model           = struct('ncc_thesame',0.95,'valid',0.5,'patchsize',[10 10],'min_win',20,'num_trees',10,'num_features',13,'thr_fern',0.5,'thr_nn',0.65,'thr_nn_valid',0.7,'fliplr',0);
opt.p_par_init      = struct('num_closest',10,'num_warps',20,'noise',5,'angle',20,'shift',0.02,'scale',0.02);
opt.p_par_update    = struct('num_closest',10,'num_warps',10,'noise',5,'angle',10,'shift',0.02,'scale',0.02);
opt.n_par           = struct('overlap',0.2,'num_patches',100,'num_synthetic',0,'noise',1);
opt.tracker         = struct('occlusion',10,'grid',10,'big_fb',10);
opt.control         = struct('maxbbox',.9,'update_detector',1,'drop_img',1,'repeat',1,'rescale',1);
opt.plot            = struct('pex',1,'nex',1,'save',0,'dt',0,'confidence',0,'target',0,'replace',0,'drawoutput',3,'draw',0,'pts',0,'help', 0,'patch_rescale',1);

%  profile on;
global tld;
tldDemo(opt);
%   profile off;
%   profile viewer;