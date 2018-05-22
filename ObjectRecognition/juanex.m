

function [bb,conf] = juanex(opt,OBJ_NAME)

global tld; % holds results and temporal variables

% INITIALIZATION ----------------------------------------------------------

opt.source = tldInitSource(opt.source); % select data source, camera/directory

figure(2); set(2,'KeyPressFcn', @handleKey); % open figure for display of results
finish = 0; function handleKey(~,~), finish = 1; end % by pressing any key, the process will exit

while 1
    source = tldInitFirstFrame(tld,opt.source,opt.model.min_win); % get initial bounding box, return 'empty' if bounding box is too small
    if ~isempty(source), opt.source = source; break; end % check size
end

tld = tldInit(opt,[]); % train initial detector and initialize the 'tld' structure
tld = tldDisplay(0,tld); % initialize display
tld.plot.pex=0;tld.plot.nex=0;tld.plot.target=0;
% RUN-TIME ----------------------------------------------------------------
a=[]; b=[]; c=[]; d=[];guar=[];stopme=0;
for i = 2:length(tld.source.idx) % for every frame
    tld = tldProcessFrame(tld,i); % process frame i
    
    if ~finish
        tldDisplay(1,tld,i); % display results on frame i
    else
        cp = tld.bb(:,i);
        if ~isnan(cp(1,1))
            tts(OBJ_NAME, 'Microsoft Sam',7 );
        end
    end
    
    
    
    

     
%     
%     if finish % finish if any key was pressed
%         if tld.source.camera
%             stoppreview(tld.source.vid);
%             closepreview(tld.source.vid);
%              close(1);
%         end
%         close(2);
%         bb = tld.bb; conf = tld.conf; % return results
%         
%         guar=input('Save? (y/n): ','s');
%         if guar=='y'
%             [a b c d]=fern(6);
%             tld.Wj=a;
%             tld.NPj=b;
%             tld.NNj=c;
%             tld.SIPj=d;
%             tld.name=input('Name: ','s');
%             eval( ['save ',tld.name,' tld']);
%         end
%         
%         
%         return;
%     end
%     
%     if tld.plot.save == 1
%         img = getframe;
%         imwrite(img.cdata,[tld.output num2str(i,'%05d') '.png']);
%     end
    
    
end

bb = tld.bb; conf = tld.conf; % return results

guar=input('Save? (y/n): ','s');
        if guar=='y'
            [a b c d]=fern(6);
            tld.Wj=a;
            tld.NPj=b;
            tld.NNj=c;
            tld.SIPj=d;
            tld.name=input('Name: ','s');
            eval( ['save ',tld.name,' tld']);
        end

end