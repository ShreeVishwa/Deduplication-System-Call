#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/namei.h>
#include <linux/time.h>
#include "params.h"
#include "flags.h"
//struct Param {
//	char* fname;
//};

asmlinkage extern long (*sysptr)(void *arg);

asmlinkage long xdedup(void *arg)
{
	struct Param* param ;//= (struct Param*)kmalloc(sizeof(struct Param),GFP_KERNEL);
	struct file *filp1;
	struct file *filp2;
	struct file *filp3;
	struct file *filp4;
	mm_segment_t oldfs1;
	mm_segment_t oldfs2;
	mm_segment_t oldfs3;
	int bytes1;
	int bytes2;
	int bytes3;
	int i;
	struct kstat stats1;
	struct kstat stats2;
	struct kstat stats3;
	char *buff3;
	char *buff4;
	int error1, error2, error3;
	int file_size = 0;
	int kern_int1,kern_int2;
	struct path path1, path2, path3, path4;
	struct dentry* link_dentry;
	int unlink_err1, link_err1;
	char* temp_name;
	int kern_temp_int1, kern_temp_int2;
	struct dentry* src;
	struct dentry* dst;
	int vfs_ret_val;
	int c;
	int kern_path_sb1, kern_path_sb2, kern_path_sb3;
	struct path path_sb1, path_sb2, path_sb3;
	int unlink_temp_err;
	struct timespec now;
	unsigned short out_mode = 0644;
	int d_flag;
	int copy_user;
	
		
	//char buff3[20];
	param = (struct Param*)arg;//kmalloc(sizeof(struct Param*),GFP_KERNEL);
	/*copy_user = copy_from_user(param,arg,sizeof(param));
	if(copy_user){
		printk("All the elements have been imported successfully\n");
		return -copy_user;
	}*/
	//strcpy(param->fname,((struct param*)arg)->fname);
	/* dummy syscall:// returns 0 for non null, -EINVAL for NULL */
	//printk("%s\n",param->fname);
	//printk("Hello I have entered into syscall\n");
	//printk("xdedup received arg %p\n", arg);
	d_flag = ( param->b1 & FLAG_D );

	if (arg == NULL){
		printk("arg is null\n");
		return -EINVAL;
	}
	else{
		//printk("first : %s\n", ((struct Param *)arg)->fname);

		if(param->f1name == NULL){
			printk("Please enter the first input file\n");
			return -EINVAL;
		}

		if(param->f2name == NULL){
			printk("Please enter the second input file\n");
			return -EINVAL;
		}

		//printk("Hello how are\n");

		if((param->b1 & FLAG_N)){
			error1 = vfs_stat(param->f1name,&stats1);
			error2 = vfs_stat(param->f2name,&stats2);
			if(error1) {
				printk("Error Code is %d\n", error1);
				printk("Input file 1 does not exist\n");
				return -error1;
			}
			if(error2){
				printk("Error Code is %d\n", error2);
				printk("Input file 2 does not exist\n");
				return -error2;
			}

			if(!S_ISREG(stats1.mode)){
				printk("Input file 1 is not a regular file\n");
				return -EBADF;
			}

			if(!S_ISREG(stats2.mode)){
				printk("Input file 2 is not a regular file\n");
				return -EBADF;
			}	
			
			if(stats1.uid.val != stats2.uid.val){
				printk("The files belong to different owners so can't dedup\n");
				return -EPERM;
			}
	
			kern_path_sb1 = kern_path(param->f1name,LOOKUP_FOLLOW,&path_sb1);
			if(kern_path_sb1 != 0){
				printk("Error getting the path for the file 1\n");
				return -kern_path_sb1;
			}
					
			kern_path_sb2 = kern_path(param->f2name,LOOKUP_FOLLOW,&path_sb2);
			if(kern_path_sb2 != 0){
				printk("Error getting the path for the file 2\n");
				return -kern_path_sb2;
			}

			if(stats1.ino == stats2.ino && memcmp(path_sb1.dentry->d_inode->i_sb->s_uuid,path_sb2.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb1.dentry->d_inode->i_sb->s_uuid))==0){
			if(d_flag){
				printk("Files are already deduped\n");
			}
			return stats1.size;
		}

			//printk("stats 1 size is %u\n",stats1.size);
			//printk("statts 2 size is %u\n",stats2.size);

			//printk("%d\n",stats1.size == stats2.size);

			if(!(stats1.size == stats2.size)){
				printk("Two input files are of different sizes. So cannot dedup\n");
			return -1000;

		}

		while(file_size < stats1.size){
			
			buff3 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL); 	
			buff4 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL);


			//printk("I am in while loop\n");

	    		filp1 = filp_open(param->f1name, O_RDONLY, 0);
			//printk("Done resding file 1\n");
    			if (!filp1 || IS_ERR(filp1)) {
				printk("Reading file 1 is failed\n");
				printk("wrapfs_read_file err %d\n", (int) PTR_ERR(filp1));
				return -(int)PTR_ERR(filp1);  // or do something else 
    			}	
			//printk("No error in file1 pointer\n");
    			// now read len bytes from offset 0 
			filp1->f_pos = file_size;// start offset 
    			oldfs1 = get_fs();
    			set_fs(KERNEL_DS);
    			bytes1 = vfs_read(filp1, buff3,4096, &filp1->f_pos);
    			set_fs(oldfs1);
			//printk("%d\n",bytes1);
			//printk("%s\n",buff3);

    			// close the file 
    			filp_close(filp1, NULL);
			filp2 = filp_open(param->f2name,O_RDONLY,0);
			//printk("Done reading file 2\n");
			if(!filp2 || IS_ERR(filp2)){
				printk("Reading file 2 is failed\n");
				printk("wrapfs_read_file err %d\n", (int)PTR_ERR(filp2));
				return -(int)PTR_ERR(filp2);
			}
			//printk("No error in file  2 pointer\n");
			filp2->f_pos = file_size;
			oldfs2 = get_fs();
			set_fs(KERNEL_DS);
			bytes2 = vfs_read(filp2,buff4,4096,&filp2->f_pos);
			set_fs(oldfs2);
			//printk("Bytes 2 :%d\n",bytes2);
			//printk("Buff 2: %s\n",buff4);

			filp_close(filp2,NULL);
			//printk("Done reading from two chunks\n");

			i = 0;
			while(*(buff3+i) != '\0' && i < 4096 && *(buff4+i) != '\0'){
				//printk("I am comapring\n");
				if(*(buff3+i) != *(buff4+i)) break;
				i++;
			}

			//printk("Done reading file\n");
			//printk("current size is %d i is %d",file_size,i);
			
			if(i < 4096 && file_size + i != stats1.size){

				if(param->b1 & FLAG_P){
					printk("The files are partially matched");
					return (file_size+i);
				}
				printk("The files are not identical\n");
				return -1001;
			}
			
			
			file_size += bytes1;
			kfree(buff4);
			kfree(buff3);
		
			//printk("Number of bytes read are %d\n",bytes1);


		}

			if(d_flag){
				printk("Succesfully read file 1\n");
				printk("Successfully read file 2\n");
				printk("The files are identical\n");
			}

			return file_size;

		} 

		
		else if((param->b1 & FLAG_P) && !(param->b1 & FLAG_N)){

			error1 = vfs_stat(param->f1name,&stats1);
			error2 = vfs_stat(param->f2name,&stats2);
			error3 = vfs_stat(param->outfile,&stats3);
			if(error1) {
				printk("Error Code is %d\n", error1);
				printk("Input file 1 does not exist\n");
				return -error1;
			}
			if(error2){
				printk("Error Code is %d\n", error2);
				printk("Input file 2 does not exist\n");
				return -error2;
			}

			if(error3 == 0){

				out_mode = stats3.mode;

				if(!(out_mode & S_IWUSR)){
					printk("Output file doesn't have write permission\n");
					return -EPERM;
				}
			
				kern_path_sb3 = kern_path(param->outfile,LOOKUP_FOLLOW,&path_sb3);
				if(kern_path_sb3 != 0){
					printk("Error getting the path\n");
					return -kern_path_sb3;
				}

				if(!S_ISREG(stats3.mode)){
					printk("Output file is not a regular file\n");
					return -1002;
				}

				if(stats1.ino == stats3.ino && memcmp(path_sb1.dentry->d_inode->i_sb->s_uuid,path_sb3.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb1.dentry->d_inode->i_sb->s_uuid))==0){
				printk("Output file is already hardlinked with the input file 1\n");
				return -1003;
				}
	
				if(stats3.ino == stats2.ino && memcmp(path_sb3.dentry->d_inode->i_sb->s_uuid,path_sb2.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb2.dentry->d_inode->i_sb->s_uuid))==0){
				printk("Output file is already hardlinked with the input file 2\n");
				return -1003;
				}


			}

			if(!S_ISREG(stats1.mode)){
				printk("Input file 1 is not a regular file\n");
				return -EBADF;
			}

			if(!S_ISREG(stats2.mode)){
				printk("Input file 2 is not a regular file\n");
				return -EBADF;
			}	
			
			if(stats1.uid.val != stats2.uid.val){
				printk("The files belong to different owners so can't dedup\n");
				return -EPERM;
			}
	
			kern_path_sb1 = kern_path(param->f1name,LOOKUP_FOLLOW,&path_sb1);
			if(kern_path_sb1 != 0){
				printk("Error getting the path for the file 1\n");
				return -kern_path_sb1;
			}
					
			kern_path_sb2 = kern_path(param->f2name,LOOKUP_FOLLOW,&path_sb2);
			if(kern_path_sb2 != 0){
				printk("Error getting the path for the file 2\n");
				return -kern_path_sb2;
			}

			/*if(stats1.ino == stats2.ino && memcmp(path_sb1.dentry->d_inode->i_sb->s_uuid,path_sb2.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb1.dentry->d_inode->i_sb->s_uuid))==0){
			printk("Files are already deduped\n");
			return 0;
		}*/

			//printk("stats 1 size is %u\n",stats1.size);
			//printk("statts 2 size is %u\n",stats2.size);
			//
		if(stats1.size == 0 && stats2.size == 0){
			printk("Both the files are newly created\n");
			if(error3 == 0) {
				printk("File already exists and the input files are empty\n");
				return 0;
			}
			filp4 = filp_open(param->outfile,O_WRONLY | O_CREAT, 0644);
			if(!filp4 || IS_ERR(filp4)){
				printk("Wrapfs_read_file_4 err %d\n", (int)PTR_ERR(filp4));
				return -(int)PTR_ERR(filp4);
			}
			return 0;
		}
		
		getnstimeofday(&now);
		if(d_flag){
			printk("File owners are same\n");
		}
	//printk("%lld\n",now.tv_nsec);
		//n = snprintf(NULL,0,"%ld",now.tv_nsec);
		temp_name = (char*)kmalloc(sizeof(char)*10,GFP_KERNEL);
		c = snprintf(temp_name,10,"%ld",now.tv_nsec);
		
		//printk("%s\n",temp_name);

		//temp_name = "tempfile.txt";
		filp1 = filp_open(param->f1name, O_RDONLY, 0);
			//printk("Done resding file 1\n");
    		if (!filp1 || IS_ERR(filp1)) {
			printk("Reading file 1 is failed\n");
			printk("wrapfs_read_file err %d\n", (int) PTR_ERR(filp1));
			return -(int)PTR_ERR(filp1);  // or do something else 
    		}
		
		filp1->f_pos = 0;
		//filp_close(filp1, NULL);
		filp2 = filp_open(param->f2name,O_RDONLY,0);
			//printk("Done reading file 2\n");
		if(!filp2 || IS_ERR(filp2)){
			printk("Reading file 2 is failed\n");
			printk("wrapfs_read_file err %d\n", (int)PTR_ERR(filp2));
			return -(int)PTR_ERR(filp2);
		}
		//printk("No error in file  2 pointer\n");

		filp2->f_pos = 0;

		filp3 = filp_open(temp_name,O_WRONLY|O_CREAT,out_mode);
			//printk("Done reading file\n");
		if(!filp3 || IS_ERR(filp3)){
			printk("wrapfs_read_file_3 err %d\n", (int)PTR_ERR(filp3));
			return -(int)PTR_ERR(filp3);
		}
		//printk("No error in file 3 pointer\n");

		filp3->f_pos = file_size;

		filp4 = filp_open(param->outfile,O_WRONLY | O_CREAT, out_mode);
		if(!filp4 || IS_ERR(filp4)){
			printk("Wrapfs_read_file_4 err %d\n", (int)PTR_ERR(filp4));
			return -(int)PTR_ERR(filp4);
		}

		if(d_flag){
			printk("File 1 has been successfully opened\n");
			printk("File 2 has been successfully opened\n");
			printk("A temporary file has been created\n");
		}

	


		while(file_size < stats1.size){
			
			buff3 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL); 	
			buff4 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL);

			//temp_name = "tempfile.txt";			

			//printk("I am in while loop\n");

	    			
			//printk("No error in file1 pointer\n");
    			// now read len bytes from offset 0 
			//filp1->f_pos = file_size;// start offset 
    			oldfs1 = get_fs();
    			set_fs(KERNEL_DS);
    			bytes1 = vfs_read(filp1, buff3, 4096, &filp1->f_pos);
    			set_fs(oldfs1);
			//printk("%d\n",bytes1);
			//printk("%s\n",buff3);

    			// close the file 
    			//filp2->f_pos = file_size;
			oldfs2 = get_fs();
			set_fs(KERNEL_DS);
			bytes2 = vfs_read(filp2,buff4,4096,&filp2->f_pos);
			set_fs(oldfs2);
			//printk("Bytes 2 :%d\n",bytes2);
			//printk("Buff 2: %s\n",buff4);

			//filp_close(filp2,NULL);
			//printk("Done reading from two chunks\n");

			

			i = 0;
			while(*(buff3+i) != '\0' && i < 4096 && *(buff4+i) != '\0'){
				//printk("I am comapring\n");
				if(*(buff3+i) != *(buff4+i)) break;
				i++;
			}

			if(i == 0 && file_size == 0){
				if(error3 == 0){
					printk("Nothing is matching in input files and keeping the existing output file\n");
				return 0;
				}
				filp4 = filp_open(param->outfile,O_WRONLY | O_CREAT, 0644);
				if(!filp4 || IS_ERR(filp4)){
					printk("Wrapfs_read_file_4 err %d\n", (int)PTR_ERR(filp4));
					return -(int)PTR_ERR(filp4);
				}
				return 0;
			}
	

			//printk("Done reading file\n");
			
				//filp3->f_pos = file_size;
			oldfs3 = get_fs();
			set_fs(KERNEL_DS);
	
			bytes3 = vfs_write(filp3,buff3,i,&filp3->f_pos);
			
			if(i != bytes3){
				printk("Error in writing the file\n");
				mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
				unlink_temp_err = vfs_unlink((path3.dentry)->d_parent->d_inode,path3.dentry,NULL);	
				mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
				printk("Temp file deleted\n");
				if(unlink_temp_err){
					printk("Unlinking the temp file failed\n");
					return -unlink_temp_err;
				}

				return -1004;
	
			}

			//filp_close(filp3,NULL);

					

			if(i < 4096 && file_size + i != stats1.size){
				file_size = file_size + i;
				break;
			}
		
			file_size += bytes1;
			kfree(buff4);
			kfree(buff3);
		
			//printk("Number of bytes read are %d\n",bytes1);


		}

			filp_close(filp3,NULL);
			filp_close(filp2,NULL);
			filp_close(filp1,NULL);

			if(d_flag){
				printk("File 1 has been successfully read in page size pf 4KB\n");
				printk("File 2 has been successfully read in page size of 4KB\n");
				printk("Successfully written to the temp file\n");
			}


			kern_temp_int1 = kern_path(temp_name,LOOKUP_FOLLOW,&path3);
			kern_temp_int2 = kern_path(param->outfile,LOOKUP_FOLLOW,&path4);

			if(kern_temp_int1){
				printk("Error in getting the path name for file 1\n");
				return -kern_temp_int1;
			}
	
			if(kern_temp_int2){
				printk("Error in getting the path name for file 2\n");
				return -kern_temp_int2;
			}

			//printk("kern_temp_1 is %d\n",kern_temp_int1);
			//printk("kern_temp_2 is %d\n",kern_temp_int2);

			mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
	
			src = lookup_one_len(temp_name,path3.dentry->d_parent,strlen(temp_name));
			mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
	
			if(IS_ERR(src)){
				printk("There is error in src lookup\n");
				return((int)PTR_ERR(src));
			}

			mutex_lock_nested(&(path4.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
	
 			dst = lookup_one_len(path4.dentry->d_iname,path4.dentry->d_parent,strlen(path4.dentry->d_iname));
			
			mutex_unlock(&(path4.dentry)->d_parent->d_inode->i_mutex);
	
			
			if(IS_ERR(dst)){
				printk("There is err in dst lookup\n");
				return ((int)PTR_ERR(dst));
			}
			
			if(d_flag){
				printk("src and dest dentries are taken\n");
			}

			lock_rename(src->d_parent,dst->d_parent);

			vfs_ret_val = vfs_rename(path3.dentry->d_parent->d_inode,src,path4.dentry->d_parent->d_inode,dst,NULL,0);
			
			unlock_rename(src->d_parent,dst->d_parent);
			//printk("vfs_rname done val is %d\n", vfs_ret_val);
			
			if(vfs_ret_val){
				printk("Rename unsuccessfull\n");
				return -vfs_ret_val;
			}

			if(d_flag){
				printk("Found the source dentry\n");
				printk("Found the destination dentry\n");
				printk("Locks have been applied successfully\n");
				printk("File has been succcessfully renamed\n");
			}
			kfree(temp_name);

			return file_size;



		}

		else if(!(param->b1 & FLAG_P) && !(param->b1 & FLAG_N)){

			error1 = vfs_stat(param->f1name,&stats1);
			error2 = vfs_stat(param->f2name,&stats2);
			error3 = vfs_stat(param->outfile,&stats3);
			if(error1) {
				printk("Error Code is %d\n", error1);
				printk("Input file 1 does not exist\n");
				return -error1;
			}
			if(error2){
				printk("Error Code is %d\n", error2);
				printk("Input file 2 does not exist\n");
				return -error2;
			}

			if(error3 == 0){
			
				kern_path_sb3 = kern_path(param->outfile,LOOKUP_FOLLOW,&path_sb2);
				if(kern_path_sb3 != 0){
					printk("Error getting the path\n");
					return -kern_path_sb3;
				}

				if(!S_ISREG(stats3.mode)){
					printk("Output file is not a regular file\n");
					return -1002;
				}

				if(stats1.ino == stats3.ino && memcmp(path_sb1.dentry->d_inode->i_sb->s_uuid,path_sb3.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb1.dentry->d_inode->i_sb->s_uuid))==0){
				printk("Output file is already hardlinked with the input file 1\n");
				return -1003;
				}
	
				if(stats3.ino == stats2.ino && memcmp(path_sb3.dentry->d_inode->i_sb->s_uuid,path_sb2.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb2.dentry->d_inode->i_sb->s_uuid))==0){
				printk("Output file is already hardlinked with the input file 2\n");
				return -1003;
				}


			}

			if(!S_ISREG(stats1.mode)){
				printk("Input file 1 is not a regular file\n");
				return -EBADF;
			}

			if(!S_ISREG(stats2.mode)){
				printk("Input file 2 is not a regular file\n");
				return -EBADF;
			}	
			
			if(stats1.uid.val != stats2.uid.val){
				printk("The files belong to different owners so can't dedup\n");
				return -EPERM;
			}
	
			kern_path_sb1 = kern_path(param->f1name,LOOKUP_FOLLOW,&path_sb1);
			if(kern_path_sb1 != 0){
				printk("Error getting the path for the file 1\n");
				return -kern_path_sb1;
			}
					
			kern_path_sb2 = kern_path(param->f2name,LOOKUP_FOLLOW,&path_sb2);
			if(kern_path_sb2 != 0){
				printk("Error getting the path for the file 2\n");
				return -kern_path_sb2;
			}

			if(stats1.ino == stats2.ino && memcmp(path_sb1.dentry->d_inode->i_sb->s_uuid,path_sb2.dentry->d_inode->i_sb->s_uuid,sizeof(path_sb1.dentry->d_inode->i_sb->s_uuid))==0){
			printk("Files are already deduped\n");
			return -1005;
		}

			if(!(stats1.size == stats2.size)){
				printk("Two input files are of different sizes. So cannot dedup\n");
			return -1000;

		}

		if(d_flag){
			printk("All the checks have been successfully performed\n");
		}

		getnstimeofday(&now);
	//printk("%lld\n",now.tv_nsec);
		//n = snprintf(NULL,0,"%ld",now.tv_nsec);*/
		temp_name = (char*)kmalloc(sizeof(char)*20,GFP_KERNEL);
		c = snprintf(temp_name,15,"%ld",now.tv_nsec);
		//printk("%s\n",temp_name);
		//temp_name = "tempfile.txt";
		//temp_name = "tempfile.txt";
		
	

		while(file_size < stats1.size){
			
			buff3 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL); 	
			buff4 = (char*)kmalloc(sizeof(char)*4096,GFP_KERNEL);


			//printk("I am in while loop\n");

	    		filp1 = filp_open(param->f1name, O_RDONLY, 0);
			//printk("Done resding file 1\n");
    			if (!filp1 || IS_ERR(filp1)) {
				printk("Reading file 1 is failed\n");
				printk("wrapfs_read_file err %d\n", (int) PTR_ERR(filp1));
				return -(int)PTR_ERR(filp1);  // or do something else 
    			}	
			//printk("No error in file1 pointer\n");
    			// now read len bytes from offset 0 
			filp1->f_pos = file_size;// start offset 
    			oldfs1 = get_fs();
    			set_fs(KERNEL_DS);
    			bytes1 = vfs_read(filp1, buff3, 4096, &filp1->f_pos);
    			set_fs(oldfs1);
			//printk("%d\n",bytes1);
			//printk("%s\n",buff3);

    			// close the file 
    			filp_close(filp1, NULL);
			filp2 = filp_open(param->f2name,O_RDONLY,0);
			//printk("Done reading file 2\n");
			if(!filp2 || IS_ERR(filp2)){
				printk("Reading file 2 is failed\n");
				printk("wrapfs_read_file err %d\n", (int)PTR_ERR(filp2));
				return -(int)PTR_ERR(filp2);
			}
			//printk("No error in file  2 pointer\n");
			filp2->f_pos = file_size;
			oldfs2 = get_fs();
			set_fs(KERNEL_DS);
			bytes2 = vfs_read(filp2,buff4,4096,&filp2->f_pos);
			set_fs(oldfs2);
			//printk("Bytes 2 :%d\n",bytes2);
			//printk("Buff 2: %s\n",buff4);

			filp_close(filp2,NULL);
			printk("Done reading from two chunks\n");

			i = 0;
			while(*(buff3+i) != '\0' && i < 4096 && *(buff4+i) != '\0'){
				//printk("I am comapring\n");
				if(*(buff3+i) != *(buff4+i)) break;
				i++;
			}

			//printk("Done reading file\n");
			//printk("current size is %d i is %d",file_size,i);
			
			filp3 = filp_open(temp_name,O_WRONLY|O_CREAT,0644);
			//printk("Done reading file\n");
			if(!filp3 || IS_ERR(filp3)){
				printk("wrapfs_read_file_3 err %d\n", (int)PTR_ERR(filp3));
				return -(int)PTR_ERR(filp3);
			}
			//printk("No error in file 3 pointer\n");
			kern_temp_int1 = kern_path(temp_name,LOOKUP_FOLLOW,&path3);

			if(kern_temp_int1){
				printk("Error in getting the path\n");
				return kern_temp_int1;
			}


			if(i < 4096 && file_size + i != stats1.size){
				
				//printk("The files are not identical\n");
				mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
				unlink_temp_err = vfs_unlink((path3.dentry)->d_parent->d_inode,path3.dentry,NULL);	
				mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
				//printk("Temp file deleted\n");
				if(unlink_temp_err){
					printk("Unlinking the temp file failed\n");
					return -unlink_temp_err;
				}

				return -1001;
			}

			filp3->f_pos = file_size;
			oldfs3 = get_fs();
			set_fs(KERNEL_DS);
	
			bytes3 = vfs_write(filp3,buff3,i,&filp3->f_pos);

			filp_close(filp3,NULL);


			
			file_size += bytes1;
			kfree(buff4);
			kfree(buff3);
		
			//printk("Number of bytes read are %d\n",bytes1);


		}

			//printk("Completed reading all bytes\n");
			if(d_flag){
				printk("Successfully read file 1\n");
				printk("Successfully read file 2\n");
				printk("Successfully written the data to the temp file\n");
			}

			kern_int2 = kern_path(param->f2name,LOOKUP_FOLLOW,&path2);
			if(kern_int2){
				printk("Error getting the path\n");
				return -1006;
			}
			mutex_lock_nested(&(path2.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
			unlink_err1 = vfs_unlink((path2.dentry)->d_parent->d_inode,path2.dentry,NULL);	
			mutex_unlock(&(path2.dentry)->d_parent->d_inode->i_mutex);
			
			if(unlink_err1){
				printk("Unlink of file 2 failed\n");
				mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
				unlink_temp_err = vfs_unlink((path3.dentry)->d_parent->d_inode,path3.dentry,NULL);	
				mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
				if(unlink_temp_err){
					printk("Unlinking with the temp file also failed\n");
					return -unlink_temp_err;
				}
				return -unlink_err1;

			}	

			//printk("unlink error is %d\n",unlink_err1);
			//
			if(d_flag){
				printk("Unlinking done successfully\n");
			}

			kern_int1 = kern_path(param->f1name,LOOKUP_FOLLOW,&path1);
			if(kern_int1){
				printk("Error in getting the path\n");
				return -1006;
			}
			mutex_lock_nested(&(path2.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
			link_dentry = lookup_one_len(path2.dentry->d_iname,path2.dentry->d_parent,strlen(path2.dentry->d_iname));
			mutex_unlock(&(path2.dentry)->d_parent->d_inode->i_mutex);
			link_err1 = vfs_link(path1.dentry,path1.dentry->d_parent->d_inode,link_dentry,NULL);
			//printk("kern_int1 : %d\n",kern_int1);
			//printk("link_err1: %d\n",link_err1);
			

			if(link_err1){
				mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
	
			src = lookup_one_len(temp_name,path3.dentry->d_parent,strlen(temp_name));
			mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
	
			if(IS_ERR(src)){
				printk("There is error in src lookup\n");
				return((int)PTR_ERR(src));
			}

			mutex_lock_nested(&(path2.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
	
 			dst = lookup_one_len(path2.dentry->d_iname,path2.dentry->d_parent,strlen(path2.dentry->d_iname));
			
			mutex_unlock(&(path2.dentry)->d_parent->d_inode->i_mutex);
	
			
			if(IS_ERR(dst)){
				printk("There is err in dst lookup\n");
				return ((int)PTR_ERR(dst));
			}
	
			printk("src and dest are done\n");

			lock_rename(src->d_parent,dst->d_parent);

			vfs_ret_val = vfs_rename(path3.dentry->d_parent->d_inode,src,path2.dentry->d_parent->d_inode,dst,NULL,0);
			
			unlock_rename(src->d_parent,dst->d_parent);
			//printk("vfs_rname done val is %d\n", vfs_ret_val);

			if(vfs_ret_val){
				
				printk("VFS_Rename Failed and the file 2 is lost\n");
				return -vfs_ret_val;
			}

			printk("File 2 is restored after linking is failed\n");
			return -link_err1;

			}

			if(d_flag){
				printk("Files have been linked successfully\n");
				printk("The generated temporary file has been unlinked\n");
				printk("Files have been succesfully deduped\n");
			}

			mutex_lock_nested(&(path3.dentry)->d_parent->d_inode->i_mutex, I_MUTEX_PARENT);
			unlink_temp_err = vfs_unlink((path3.dentry)->d_parent->d_inode,path3.dentry,NULL);	
			mutex_unlock(&(path3.dentry)->d_parent->d_inode->i_mutex);
			if(unlink_temp_err){
				printk("Unlinking with the temp file alos failed\n");
				return -unlink_temp_err;
			}
			
			return file_size;

		
		}
		
		return 0;

	}
}

static int __init init_sys_xdedup(void)
{
	printk("installed new sys_xdedup module\n");
	if (sysptr == NULL)

		sysptr = xdedup;
	return 0;
}
static void  __exit exit_sys_xdedup(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk("removed sys_xdedup module\n");
}
module_init(init_sys_xdedup);
module_exit(exit_sys_xdedup);
MODULE_LICENSE("GPL");
